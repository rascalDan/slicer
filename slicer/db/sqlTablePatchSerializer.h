#ifndef SLICER_DB_SQLTABLEPATCHSERIALIZER_H
#define SLICER_DB_SQLTABLEPATCHSERIALIZER_H

#include "modelParts.h"
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

		void Serialize(Slicer::ModelPartForRootPtr) override;

	private:
		void createTemporaryTable();
		void dropTemporaryTable();

		DB::Connection * const db;
		DB::TablePatch & tablePatch;
	};
}

#endif
