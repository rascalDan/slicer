#ifndef SLICER_DB_SQLTABLEPATCHSERIALIZER_H
#define SLICER_DB_SQLTABLEPATCHSERIALIZER_H

#include <slicer/serializer.h>
#include <tablepatch.h>

namespace Slicer {
	class DLL_PUBLIC SqlTablePatchSerializer : public Slicer::Serializer {
		public:
			SqlTablePatchSerializer(DB::Connection *, DB::TablePatch &);
			~SqlTablePatchSerializer();

			virtual void Serialize(Slicer::ModelPartForRootPtr) override;

		private:
			void createTemporaryTable();
			void dropTemporaryTable();

			DB::Connection * db;
			DB::TablePatch & tablePatch;
	};
}

#endif

