#include "sqlInsertSerializer.h"
#include <common.h>
#include <sqlExceptions.h>
#include "sqlBinder.h"
#include "sqlCommon.h"
#include <compileTimeFormatter.h>
#include <modifycommand.h>
#include <slicer/metadata.h>
#include <boost/numeric/conversion/cast.hpp>
#include <functional>

namespace Slicer {
	using namespace std::placeholders;

	SqlInsertSerializer::SqlInsertSerializer(DB::Connection * const c, std::string t) :
		connection(c),
		tableName(std::move(t))
	{
	}

	void
	SqlInsertSerializer::Serialize(Slicer::ModelPartForRootPtr mp)
	{
		switch (mp->GetType()) {
			case Slicer::mpt_Sequence:
				mp->OnEachChild(std::bind(&SqlInsertSerializer::SerializeSequence, this, _2));
				return;
			case Slicer::mpt_Complex:
				mp->OnEachChild(std::bind(&SqlInsertSerializer::SerializeObject, this, _2));
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
		int paramNo = 0;
		cmp->OnEachChild(std::bind(&SqlInsertSerializer::bindObjectAndExecuteField, this, std::ref(paramNo), ins, _2, _3));
		ins->execute();
	}

	class IdSave : public Slicer::ValueSource {
		public:
			explicit IdSave(DB::Connection * const c) :
				connection(c)
			{
			}

#define NonNumType(T) \
			/* NOLINTNEXTLINE(bugprone-macro-parentheses) */ \
			void set(T &) const override { throw UnsuitableIdFieldType(#T); }

#define NumType(T) \
			/* NOLINTNEXTLINE(bugprone-macro-parentheses) */ \
			void set(T & v) const override { v = boost::numeric_cast<T>(connection->insertId()); }

			NonNumType(bool);
			NonNumType(std::string);

			NumType(Ice::Byte);
			NumType(Ice::Short);
			NumType(Ice::Int);
			NumType(Ice::Long);
			NumType(Ice::Float);
			NumType(Ice::Double);

		private:
			DB::Connection * const connection;
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
	SqlInsertSerializer::bindObjectAndExecuteField(int & paramNo, DB::ModifyCommand * ins, const Slicer::ModelPartPtr & cmp, const HookCommon * h) const
	{
		if (isBind(h)) {
			if (!cmp->GetValue(SqlBinder(*ins, paramNo))) {
				ins->bindNull(paramNo);
			}
			paramNo++;
		}
	}

	void
	SqlAutoIdInsertSerializer::bindObjectAndExecuteField(int & paramNo, DB::ModifyCommand * ins, const Slicer::ModelPartPtr & cmp, const HookCommon * h) const
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
		int fieldNo = 0;
		mp->OnEachChild(std::bind(&SqlInsertSerializer::createInsertField, this, std::ref(fieldNo), std::ref(insert), _1, _3));
		insert << ") VALUES (";
		for (; fieldNo > 1; --fieldNo) {
			insert << "?, ";
		}
		insert << "?)";
		return connection->modify(insert.str());
	}

	void
	SqlInsertSerializer::createInsertField(int & fieldNo, std::ostream & insert, const std::string & name, const HookCommon * h) const
	{
		if (isBind(h)) {
			if (fieldNo++) {
				insert << ',';
			}
			insert << name;
		}
	}

	void
	SqlAutoIdInsertSerializer::createInsertField(int & fieldNo, std::ostream & insert, const std::string & name, const HookCommon * h) const
	{
		if (isNotAuto(h)) {
			if (fieldNo++) {
				insert << ',';
			}
			insert << name;
		}
	}
}

