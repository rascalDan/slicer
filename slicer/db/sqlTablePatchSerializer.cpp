#include "sqlTablePatchSerializer.h"
#include "sqlCommon.h"
#include "sqlInsertSerializer.h"
#include <compileTimeFormatter.h>
#include <connection.h>
#include <memory>
#include <scopeExit.h>
#include <string>
#include <tablepatch.h>

namespace Slicer {
	AdHocFormatter(ttname, "slicer_tmp_%?");
	SqlTablePatchSerializer::SqlTablePatchSerializer(DB::Connection * const db, DB::TablePatch & tp) :
		db(db), tablePatch(tp)
	{
		tablePatch.src = ttname::get(this);
	}

	void
	SqlTablePatchSerializer::Serialize(Slicer::ModelPartForRootPtr mpr)
	{
		tablePatch.pk.clear();
		tablePatch.cols.clear();

		createTemporaryTable();
		AdHoc::ScopeExit tidy([this] {
			dropTemporaryTable();
		});

		SqlInsertSerializer ins(db, tablePatch.src);
		ins.Serialize(mpr);

		auto mp = mpr->GetContainedModelPart();
		mp->OnEachChild([this](const auto & name, const auto &, const auto & h) {
			if (isPKey(h)) {
				tablePatch.pk.insert(name);
			}
		});
		mp->OnEachChild([this](const auto & name, const auto &, const auto & h) {
			if (isBind(h)) {
				tablePatch.cols.insert(name);
			}
		});

		db->patchTable(&tablePatch);
	}

	AdHocFormatter(createTmpTable, "CREATE TEMPORARY TABLE %? AS SELECT * FROM %? WHERE 1 = 0");
	void
	SqlTablePatchSerializer::createTemporaryTable()
	{
		db->execute(createTmpTable::get(tablePatch.src, tablePatch.dest));
	}

	AdHocFormatter(dropTmpTable, "DROP TABLE %?");
	void
	SqlTablePatchSerializer::dropTemporaryTable()
	{
		db->execute(dropTmpTable::get(tablePatch.src));
	}
}
