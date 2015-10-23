#include "sqlUpdateSerializer.h"
#include "exceptions.h"
#include "sqlBinder.h"
#include <buffer.h>
#include <modifycommand.h>
#include <slicer/metadata.h>

namespace Slicer {
	NoRowsFound::NoRowsFound() : std::runtime_error("No rows found") { }

	const std::string md_pkey = "db:pkey";

	SqlUpdateSerializer::SqlUpdateSerializer(const DB::Connection * c, const std::string & t) :
		connection(c),
		tableName(t)
	{
	}

	void
	SqlUpdateSerializer::Serialize(Slicer::ModelPartPtr mp)
	{
		switch (mp->GetType()) {
			case Slicer::mpt_Sequence:
				mp->OnEachChild(boost::bind(&SqlUpdateSerializer::SerializeSequence, this, _2));
				return;
			case Slicer::mpt_Complex:
				mp->OnEachChild(boost::bind(&SqlUpdateSerializer::SerializeObject, this, _2));
				return;
			default:
				throw UnsupportedModelType();
		}
	}

	void
	SqlUpdateSerializer::SerializeObject(Slicer::ModelPartPtr mp) const
	{
		auto ins = createUpdate(mp);
		bindObjectAndExecute(mp, ins.get());
	}

	void
	SqlUpdateSerializer::SerializeSequence(Slicer::ModelPartPtr mp) const
	{
		ModifyPtr ins;
		mp->OnEachChild([&ins, this](const std::string &, ModelPartPtr cmp, HookCommonPtr) {
				if (!ins) {
					ins = createUpdate(cmp);
				}
				bindObjectAndExecute(cmp, ins.get());
			});
	}

	void
	SqlUpdateSerializer::bindObjectAndExecute(Slicer::ModelPartPtr cmp, DB::ModifyCommand * upd)
	{
		int paramNo = 0;
		cmp->OnEachChild([&upd, &paramNo](const std::string &, ModelPartPtr cmp, HookCommonPtr h) {
				if (metaDataFlagNotSet(h->GetMetadata(), md_pkey)) {
					cmp->GetValue(new SqlBinder(*upd, paramNo++));
				}
			});
		cmp->OnEachChild([&upd, &paramNo](const std::string &, ModelPartPtr cmp, HookCommonPtr h) {
				if (metaDataFlagSet(h->GetMetadata(), md_pkey)) {
					cmp->GetValue(new SqlBinder(*upd, paramNo++));
				}
			});
		if (upd->execute() == 0) {
			throw NoRowsFound();
		}
	}

	SqlUpdateSerializer::ModifyPtr
	SqlUpdateSerializer::createUpdate(Slicer::ModelPartPtr mp) const
	{
		AdHoc::Buffer update;
		update.appendbf("UPDATE %s SET ", tableName);
		int fieldNo = 0;
		mp->OnEachChild([&update, &fieldNo]( const std::string & name, ModelPartPtr, HookCommonPtr h) {
				if (metaDataFlagNotSet(h->GetMetadata(), md_pkey)) {
					if (fieldNo++) {
						update.append(", ");
					}
					update.appendbf("%s = ?", name);
				}
			});
		update.append(" WHERE ", AdHoc::Buffer::Use);
		fieldNo = 0;
		mp->OnEachChild([&update, &fieldNo]( const std::string & name, ModelPartPtr, HookCommonPtr h) {
				if (metaDataFlagSet(h->GetMetadata(), md_pkey)) {
					if (fieldNo++) {
						update.append(" AND ");
					}
					update.appendbf("%s = ?", name);
				}
			});
		return ModifyPtr(connection->newModifyCommand(update));
	}
}

