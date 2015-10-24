#include "sqlInsertSerializer.h"
#include "exceptions.h"
#include "sqlBinder.h"
#include <buffer.h>
#include <modifycommand.h>
#include <slicer/metadata.h>
#include <boost/numeric/conversion/cast.hpp>

namespace Slicer {
	const std::string md_auto = "db:auto";

	SqlInsertSerializer::SqlInsertSerializer(const DB::Connection * c, const std::string & t) :
		connection(c),
		tableName(t)
	{
	}

	void
	SqlInsertSerializer::Serialize(Slicer::ModelPartPtr mp)
	{
		switch (mp->GetType()) {
			case Slicer::mpt_Sequence:
				mp->OnEachChild(boost::bind(&SqlInsertSerializer::SerializeSequence, this, _2));
				return;
			case Slicer::mpt_Complex:
				mp->OnEachChild(boost::bind(&SqlInsertSerializer::SerializeObject, this, _2));
				return;
			default:
				throw UnsupportedModelType();
		}
	}

	void
	SqlInsertSerializer::SerializeObject(Slicer::ModelPartPtr mp) const
	{
		auto ins = createInsert(mp);
		bindObjectAndExecute(mp, ins.get());
	}

	void
	SqlInsertSerializer::SerializeSequence(Slicer::ModelPartPtr mp) const
	{
		ModifyPtr ins;
		mp->OnEachChild([&ins, this](const std::string &, ModelPartPtr cmp, HookCommonPtr) {
				if (!ins) {
					ins = createInsert(cmp);
				}
				bindObjectAndExecute(cmp, ins.get());
			});
	}

	void
	SqlInsertSerializer::bindObjectAndExecute(Slicer::ModelPartPtr cmp, DB::ModifyCommand * ins) const
	{
		int paramNo = 0;
		cmp->OnEachChild(boost::bind(&SqlInsertSerializer::bindObjectAndExecuteField, this, boost::ref(paramNo), ins, _2, _3));
		ins->execute();
	}

	class IdSave : public Slicer::ValueSource {
		public:
			IdSave(const DB::Connection * c) :
				connection(c)
			{
			}

#define NonNumType(T) \
			void set(T &) const override { throw std::runtime_error("Can't store Id in " #T " type field"); }

#define NumType(T) \
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
			const DB::Connection * connection;
	};

	void
	SqlFetchIdInsertSerializer::bindObjectAndExecute(Slicer::ModelPartPtr cmp, DB::ModifyCommand * ins) const
	{
		SqlAutoIdInsertSerializer::bindObjectAndExecute(cmp, ins);
		cmp->OnEachChild([&ins, this](const std::string &, ModelPartPtr cmp, HookCommonPtr h) {
				if (metaDataFlagSet(h->GetMetadata(), md_auto)) {
					cmp->SetValue(new IdSave(connection));
				}
			});
	}

	void
	SqlInsertSerializer::bindObjectAndExecuteField(int & paramNo, DB::ModifyCommand * ins, Slicer::ModelPartPtr cmp, HookCommonPtr) const
	{
		if (cmp) {
			cmp->GetValue(new SqlBinder(*ins, paramNo++));
		}
		else {
			ins->bindNull(paramNo++);
		}
	}

	void
	SqlAutoIdInsertSerializer::bindObjectAndExecuteField(int & paramNo, DB::ModifyCommand * ins, Slicer::ModelPartPtr cmp, HookCommonPtr h) const
	{
		if (metaDataFlagNotSet(h->GetMetadata(), md_auto)) {
			SqlInsertSerializer::bindObjectAndExecuteField(paramNo, ins, cmp, h);
		}
	}

	SqlInsertSerializer::ModifyPtr
	SqlInsertSerializer::createInsert(Slicer::ModelPartPtr mp) const
	{
		AdHoc::Buffer insert;
		insert.appendbf("INSERT INTO %s(", tableName);
		int fieldNo = 0;
		mp->OnEachChild(boost::bind(&SqlInsertSerializer::createInsertField, this, boost::ref(fieldNo), boost::ref(insert), _1, _3));
		insert.append(") VALUES (", AdHoc::Buffer::Use);
		for (; fieldNo > 1; --fieldNo) {
			insert.append("?, ");
		}
		insert.append("?)");
		return ModifyPtr(connection->newModifyCommand(insert));
	}

	void
	SqlInsertSerializer::createInsertField(int & fieldNo, AdHoc::Buffer & insert, const std::string & name, HookCommonPtr) const
	{
		if (fieldNo++) {
			insert.append(", ");
		}
		insert.append(name);
	}

	void
	SqlAutoIdInsertSerializer::createInsertField(int & fieldNo, AdHoc::Buffer & insert, const std::string & name, HookCommonPtr h) const
	{
		if (metaDataFlagNotSet(h->GetMetadata(), md_auto)) {
			if (fieldNo++) {
				insert.append(", ");
			}
			insert.append(name);
		}
	}
}

