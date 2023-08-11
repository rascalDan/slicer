#include "sqlInsertSerializer.h"
#include "common.h"
#include "sqlBinder.h"
#include "sqlCommon.h"
#include "sqlExceptions.h"
#include <Ice/Config.h>
#include <boost/numeric/conversion/cast.hpp>
#include <command_fwd.h>
#include <compileTimeFormatter.h>
#include <connection.h>
#include <functional>
#include <memory>
#include <modifycommand.h>
#include <slicer/modelParts.h>
#include <utility>

namespace Slicer {
	SqlInsertSerializer::SqlInsertSerializer(DB::Connection * const c, std::string t) :
		connection(c), tableName(std::move(t))
	{
	}

	void
	SqlInsertSerializer::Serialize(ModelPartForRootParam mp)
	{
		switch (mp->GetType()) {
			case Slicer::ModelPartType::Sequence:
				mp->OnEachChild([this](auto &&, auto && PH2, auto &&) {
					SerializeSequence(PH2);
				});
				return;
			case Slicer::ModelPartType::Complex:
				mp->OnEachChild([this](auto &&, auto && PH2, auto &&) {
					SerializeObject(PH2);
				});
				return;
			default:
				throw UnsupportedModelType();
		}
	}

	void
	SqlInsertSerializer::SerializeObject(ModelPartParam mp) const
	{
		auto ins = createInsert(mp);
		bindObjectAndExecute(mp, ins.get());
	}

	void
	SqlInsertSerializer::SerializeSequence(ModelPartParam mp) const
	{
		auto ins = createInsert(mp->GetContainedModelPart());
		mp->OnEachChild([&ins, this](auto &&, auto && cmp, auto &&) {
			bindObjectAndExecute(cmp, ins.get());
		});
	}

	void
	SqlInsertSerializer::bindObjectAndExecute(ModelPartParam cmp, DB::ModifyCommand * ins) const
	{
		unsigned int paramNo = 0;
		cmp->OnEachChild([this, &paramNo, ins](auto &&, auto && PH2, auto && PH3) {
			bindObjectAndExecuteField(paramNo, ins, PH2, PH3);
		});
		ins->execute();
	}

	class IdSave : public Slicer::ValueSource {
	public:
		explicit IdSave(DB::Connection * const c) : connection(c) { }

#define NumType(T) \
	void set(T & v) const override \
	{ \
		doSet(v, #T); \
	}

		NumType(bool);
		NumType(std::string);
		NumType(Ice::Byte);
		NumType(Ice::Short);
		NumType(Ice::Int);
		NumType(Ice::Long);
		NumType(Ice::Float);
		NumType(Ice::Double);

	private:
		DB::Connection * const connection;

		template<typename T>
		inline void
		doSet([[maybe_unused]] T & v, [[maybe_unused]] const char * Tname) const
		{
			if constexpr (std::is_integral_v<T>) {
				v = boost::numeric_cast<T>(connection->insertId());
			}
			else {
				throw UnsuitableIdFieldType(Tname);
			}
		}
	};

	void
	SqlFetchIdInsertSerializer::bindObjectAndExecute(ModelPartParam mp, DB::ModifyCommand * ins) const
	{
		SqlAutoIdInsertSerializer::bindObjectAndExecute(mp, ins);
		mp->OnEachChild([this](auto &&, auto && cmp, auto && h) {
			if (isAuto(h)) {
				cmp->SetValue(IdSave(connection));
			}
		});
	}

	void
	SqlInsertSerializer::bindObjectAndExecuteField(
			unsigned int & paramNo, DB::ModifyCommand * ins, ModelPartParam cmp, const HookCommon * h) const
	{
		if (isBind(h)) {
			if (!cmp->GetValue(SqlBinder(*ins, paramNo))) {
				ins->bindNull(paramNo);
			}
			paramNo++;
		}
	}

	void
	SqlAutoIdInsertSerializer::bindObjectAndExecuteField(
			unsigned int & paramNo, DB::ModifyCommand * ins, ModelPartParam cmp, const HookCommon * h) const
	{
		if (isNotAuto(h)) {
			SqlInsertSerializer::bindObjectAndExecuteField(paramNo, ins, cmp, h);
		}
	}

	DB::ModifyCommandPtr
	SqlInsertSerializer::createInsert(ModelPartParam mp) const
	{
		using namespace AdHoc::literals;
		std::stringstream insert;
		"INSERT INTO %?("_fmt(insert, tableName);
		unsigned int fieldNo = 0;
		mp->OnEachChild([this, &fieldNo, &insert](auto && PH1, auto &&, auto && PH3) {
			createInsertField(fieldNo, insert, PH1, PH3);
		});
		insert << ") VALUES (";
		for (; fieldNo > 1; --fieldNo) {
			insert << "?, ";
		}
		insert << "?)";
		return connection->modify(std::move(insert).str());
	}

	void
	SqlInsertSerializer::createInsertField(
			unsigned int & fieldNo, std::ostream & insert, const std::string & name, const HookCommon * h) const
	{
		if (isBind(h)) {
			if (fieldNo++) {
				insert << ',';
			}
			insert << name;
		}
	}

	void
	SqlAutoIdInsertSerializer::createInsertField(
			unsigned int & fieldNo, std::ostream & insert, const std::string & name, const HookCommon * h) const
	{
		if (isNotAuto(h)) {
			if (fieldNo++) {
				insert << ',';
			}
			insert << name;
		}
	}
}
