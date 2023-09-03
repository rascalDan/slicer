#pragma once

#include <slicer/modelParts.h>
#include <slicer/serializer.h>
#include <visibility.h>

namespace DB {
	class Connection;
	class TablePatch;
}

namespace Slicer {
	class DLL_PUBLIC SqlTablePatchSerializer : public Slicer::Serializer {
	public:
		SqlTablePatchSerializer(DB::Connection * const, DB::TablePatch &);

		void Serialize(ModelPartForRootParam) override;

	private:
		void createTemporaryTable();
		void dropTemporaryTable();

		DB::Connection * const db;
		DB::TablePatch & tablePatch;
	};
}
