#include "sqlUpdateSerializer.h"
#include <sqlExceptions.h>
#include <common.h>
#include "sqlBinder.h"
#include "sqlCommon.h"
#include <compileTimeFormatter.h>
#include <modifycommand.h>
#include <slicer/metadata.h>
#include <functional>

namespace Slicer {
	using namespace std::placeholders;

	SqlUpdateSerializer::SqlUpdateSerializer(DB::Connection * const c, std::string t) :
		connection(c),
		tableName(std::move(t))
	{
	}

	void
	SqlUpdateSerializer::Serialize(Slicer::ModelPartForRootPtr mp)
	{
		switch (mp->GetType()) {
			case Slicer::mpt_Sequence:
				mp->OnEachChild(std::bind(&SqlUpdateSerializer::SerializeSequence, this, _2));
				return;
			case Slicer::mpt_Complex:
				mp->OnEachChild(std::bind(&SqlUpdateSerializer::SerializeObject, this, _2));
				return;
			default:
				throw UnsupportedModelType();
		}
	}

	void
	SqlUpdateSerializer::SerializeObject(const Slicer::ModelPartPtr & mp) const
	{
		auto ins = createUpdate(mp);
		bindObjectAndExecute(mp, ins.get());
	}

	void
	SqlUpdateSerializer::SerializeSequence(const Slicer::ModelPartPtr & mp) const
	{
		auto ins = createUpdate(mp->GetContainedModelPart());
		mp->OnEachChild([&ins](const std::string &, const ModelPartPtr & cmp, const HookCommon *) {
				bindObjectAndExecute(cmp, ins.get());
			});
	}

	void
	SqlUpdateSerializer::bindObjectAndExecute(const Slicer::ModelPartPtr & cmp, DB::ModifyCommand * upd)
	{
		int paramNo = 0;
		cmp->OnEachChild([&upd, &paramNo](const std::string &, const ModelPartPtr & cmp, const HookCommon * h) {
			if (isValue(h)) {
				if (!cmp->GetValue(SqlBinder(*upd, paramNo))) {
					upd->bindNull(paramNo);
				}
				paramNo++;
			}
		});
		cmp->OnEachChild([&upd, &paramNo](const std::string &, const ModelPartPtr & cmp, const HookCommon * h) {
			if (isPKey(h)) {
				cmp->GetValue(SqlBinder(*upd, paramNo++));
			}
		});
		if (upd->execute() == 0) {
			throw NoRowsFound();
		}
	}

	DB::ModifyCommandPtr
	SqlUpdateSerializer::createUpdate(const Slicer::ModelPartPtr & mp) const
	{
		using namespace AdHoc::literals;
		std::stringstream update;
		"UPDATE %? SET "_fmt(update, tableName);
		int fieldNo = 0;
		mp->OnEachChild([&update, &fieldNo]( const std::string & name, const ModelPartPtr &, const HookCommon * h) {
			if (isValue(h)) {
				if (fieldNo++) {
					update << ", ";
				}
				"%? = ?"_fmt(update, name);
			}
		});
		update << " WHERE ";
		fieldNo = 0;
		mp->OnEachChild([&update, &fieldNo]( const std::string & name, const ModelPartPtr &, const HookCommon * h) {
			if (isPKey(h)) {
				if (fieldNo++) {
					update << " AND ";
				}
				"%? = ?"_fmt(update, name);
			}
		});
		return connection->modify(update.str());
	}
}

