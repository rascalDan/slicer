#include "sqlTablePatchSerializer.h"
#include "sqlInsertSerializer.h"
#include "sqlCommon.h"
#include <slicer/metadata.h>
#include <compileTimeFormatter.h>
#include <scopeExit.h>

namespace Slicer {
	AdHocFormatter(ttname, "slicer_tmp_%?");
	SqlTablePatchSerializer::SqlTablePatchSerializer(DB::Connection * db, DB::TablePatch & tp) :
		db(db),
		tablePatch(tp)
	{
		tablePatch.src = ttname::get(this);
	}

	SqlTablePatchSerializer::~SqlTablePatchSerializer()
	{
	}

	void
	SqlTablePatchSerializer::Serialize(Slicer::ModelPartForRootPtr mpr)
	{
		tablePatch.pk.clear();
		tablePatch.cols.clear();

		createTemporaryTable();
		AdHoc::ScopeExit tidy(boost::bind(&SqlTablePatchSerializer::dropTemporaryTable, this));

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

