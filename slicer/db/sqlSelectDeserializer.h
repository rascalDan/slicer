#ifndef SLICER_DB_SQLSELECTDESERIALIZER_H
#define SLICER_DB_SQLSELECTDESERIALIZER_H

#include <selectcommand.h>
#include <slicer/serializer.h>
#include <visibility.h>

namespace Slicer {
	class DLL_PUBLIC SqlSelectDeserializer : public Slicer::Deserializer {
	public:
		explicit SqlSelectDeserializer(
				DB::SelectCommand *, std::optional<std::string> typeIdCol = std::optional<std::string>());

		void Deserialize(Slicer::ModelPartForRootPtr) override;

	protected:
		void DLL_PRIVATE DeserializeSimple(const Slicer::ModelPartPtr &);
		void DLL_PRIVATE DeserializeObject(const Slicer::ModelPartPtr &);
		void DLL_PRIVATE DeserializeSequence(const Slicer::ModelPartPtr &);
		void DLL_PRIVATE DeserializeRow(const Slicer::ModelPartPtr &);

		DB::SelectCommand * cmd;
		unsigned int columnCount;
		std::optional<std::string> typeIdColName;
		std::optional<unsigned int> typeIdColIdx;
	};
}

#endif
