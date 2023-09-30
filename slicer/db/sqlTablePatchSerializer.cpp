#include "sqlTablePatchSerializer.h"
#include "sqlCommon.h"
#include "sqlInsertSerializer.h"
#include <compileTimeFormatter.h>
#include <connection.h>
#include <scopeExit.h>
#include <slicer/modelParts.h>
#include <string>
#include <tablepatch.h>

namespace Slicer {
	AdHocFormatter(ttname, "slicer_tmp_%?");

	SqlTablePatchSerializer::SqlTablePatchSerializer(DB::Connection * const d, DB::TablePatch & tp) :
		db(d), tablePatch(tp)
	{
		tablePatch.src = ttname::get(this);
	}

	void
	SqlTablePatchSerializer::Serialize(ModelPartForRootParam mpr)
	{
		tablePatch.pk.clear();
		tablePatch.cols.clear();

		createTemporaryTable();
		AdHoc::ScopeExit tidy([this] {
			dropTemporaryTable();
		});

		SqlInsertSerializer ins(db, tablePatch.src);
		ins.Serialize(mpr);

		mpr->OnContained([this](auto && mp) {
			mp->OnEachChild([this](const auto & name, const auto &, const auto & h) {
				if (isPKey(h)) {
					tablePatch.pk.insert(name);
				}
				if (isBind(h)) {
					tablePatch.cols.insert(name);
				}
			});
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
