#include "sqlInsertSerializer.h"
#include "sqlBinder.h"
#include "sqlCommon.h"
#include <boost/numeric/conversion/cast.hpp>
#include <common.h>
#include <compileTimeFormatter.h>
#include <functional>
#include <modifycommand.h>
#include <slicer/metadata.h>
#include <sqlExceptions.h>
#include <type_traits>

namespace Slicer {
	using namespace std::placeholders;

	SqlInsertSerializer::SqlInsertSerializer(DB::Connection * const c, std::string t) :
		connection(c), tableName(std::move(t))
	{
	}

	void
	SqlInsertSerializer::Serialize(Slicer::ModelPartForRootPtr mp)
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
	SqlInsertSerializer::SerializeObject(const Slicer::ModelPartPtr & mp) const
	{
		auto ins = createInsert(mp);
		bindObjectAndExecute(mp, ins.get());
	}

	void
	SqlInsertSerializer::SerializeSequence(const Slicer::ModelPartPtr & mp) const
	{
		auto ins = createInsert(mp->GetContainedModelPart());
		mp->OnEachChild([&ins, this](const std::string &, const ModelPartPtr & cmp, const HookCommon *) {
			bindObjectAndExecute(cmp, ins.get());
		});
	}

	void
	SqlInsertSerializer::bindObjectAndExecute(const Slicer::ModelPartPtr & cmp, DB::ModifyCommand * ins) const
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
	SqlFetchIdInsertSerializer::bindObjectAndExecute(const Slicer::ModelPartPtr & cmp, DB::ModifyCommand * ins) const
	{
		SqlAutoIdInsertSerializer::bindObjectAndExecute(cmp, ins);
		cmp->OnEachChild([this](const std::string &, const ModelPartPtr & cmp, const HookCommon * h) {
			if (isAuto(h)) {
				cmp->SetValue(IdSave(connection));
			}
		});
	}

	void
	SqlInsertSerializer::bindObjectAndExecuteField(unsigned int & paramNo, DB::ModifyCommand * ins,
			const Slicer::ModelPartPtr & cmp, const HookCommon * h) const
	{
		if (isBind(h)) {
			if (!cmp->GetValue(SqlBinder(*ins, paramNo))) {
				ins->bindNull(paramNo);
			}
			paramNo++;
		}
	}

	void
	SqlAutoIdInsertSerializer::bindObjectAndExecuteField(unsigned int & paramNo, DB::ModifyCommand * ins,
			const Slicer::ModelPartPtr & cmp, const HookCommon * h) const
	{
		if (isNotAuto(h)) {
			SqlInsertSerializer::bindObjectAndExecuteField(paramNo, ins, cmp, h);
		}
	}

	DB::ModifyCommandPtr
	SqlInsertSerializer::createInsert(const Slicer::ModelPartPtr & mp) const
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
		return connection->modify(insert.str());
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
