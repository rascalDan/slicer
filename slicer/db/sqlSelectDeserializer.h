#ifndef SLICER_DB_SQLSELECTDESERIALIZER_H
#define SLICER_DB_SQLSELECTDESERIALIZER_H

#include <slicer/serializer.h>
#include <selectcommand.h>
#include <visibility.h>

namespace Slicer {
	class DLL_PUBLIC SqlSelectDeserializer : public Slicer::Deserializer {
		public:
			SqlSelectDeserializer(DB::SelectCommand &, IceUtil::Optional<std::string> typeIdCol = IceUtil::Optional<std::string>());

			virtual void Deserialize(Slicer::ModelPartPtr) override;

		protected:
			void DLL_PRIVATE DeserializeSimple(Slicer::ModelPartPtr);
			void DLL_PRIVATE DeserializeObject(Slicer::ModelPartPtr);
			void DLL_PRIVATE DeserializeSequence(Slicer::ModelPartPtr);
			void DLL_PRIVATE DeserializeRow(Slicer::ModelPartPtr);

			DB::SelectCommand & cmd;
			unsigned int columnCount;
			IceUtil::Optional<std::string> typeIdColName;
			IceUtil::Optional<unsigned int> typeIdColIdx;
	};
}

#endif

