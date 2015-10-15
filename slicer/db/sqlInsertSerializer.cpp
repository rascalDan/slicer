#include "sqlInsertSerializer.h"
#include "exceptions.h"
#include "sqlBinder.h"
#include <buffer.h>
#include <modifycommand.h>

namespace Slicer {
	SqlInsertSerializer::SqlInsertSerializer(DB::Connection * c, const std::string & t) :
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
		int paramNo = 0;
		mp->OnEachChild([&ins, &paramNo](const std::string &, ModelPartPtr cmp, HookCommonPtr) {
				cmp->GetValue(new SqlBinder(*ins, paramNo++));
			});
		ins->execute();
	}

	void
	SqlInsertSerializer::SerializeSequence(Slicer::ModelPartPtr mp) const
	{
		ModifyPtr ins;
		mp->OnEachChild([&ins, this](const std::string &, ModelPartPtr cmp, HookCommonPtr) {
				if (!ins) {
					ins = createInsert(cmp);
				}
				int paramNo = 0;
				cmp->OnEachChild([&ins, &paramNo](const std::string &, ModelPartPtr cmp, HookCommonPtr) {
						cmp->GetValue(new SqlBinder(*ins, paramNo++));
					});
				ins->execute();
			});
	}

	SqlInsertSerializer::ModifyPtr
	SqlInsertSerializer::createInsert(Slicer::ModelPartPtr mp) const
	{
		AdHoc::Buffer insert;
		insert.appendbf("INSERT INTO %s(", tableName);
		int fieldNo = 0;
		mp->OnEachChild([&insert, &fieldNo]( const std::string & name, ModelPartPtr, HookCommonPtr) {
				if (fieldNo++) {
					insert.append(", ");
				}
				insert.append(name);
			});
		insert.append(") VALUES (", AdHoc::Buffer::Use);
		for (; fieldNo > 1; --fieldNo) {
			insert.append("?, ");
		}
		insert.append("?)");
		return ModifyPtr(connection->newModifyCommand(insert));
	}
}

