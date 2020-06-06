#ifndef SLICER_DB_SQLSELECTDESERIALIZER_H
#define SLICER_DB_SQLSELECTDESERIALIZER_H

#include <slicer/serializer.h>
#include <selectcommand.h>
#include <visibility.h>

namespace Slicer {
	class DLL_PUBLIC SqlSelectDeserializer : public Slicer::Deserializer {
		public:
			SqlSelectDeserializer(DB::SelectCommand *, Ice::optional<std::string> typeIdCol = Ice::optional<std::string>());

			void Deserialize(Slicer::ModelPartForRootPtr) override;

		protected:
			void DLL_PRIVATE DeserializeSimple(const Slicer::ModelPartPtr &);
			void DLL_PRIVATE DeserializeObject(const Slicer::ModelPartPtr &);
			void DLL_PRIVATE DeserializeSequence(const Slicer::ModelPartPtr &);
			void DLL_PRIVATE DeserializeRow(const Slicer::ModelPartPtr &);

			DB::SelectCommand * cmd;
			unsigned int columnCount;
			Ice::optional<std::string> typeIdColName;
			Ice::optional<unsigned int> typeIdColIdx;
	};
}

#endif

