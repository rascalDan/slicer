#ifndef SLICER_DB_SQLSELECTDESERIALIZER_H
#define SLICER_DB_SQLSELECTDESERIALIZER_H

#include <optional>
#include <slicer/modelParts.h>
#include <slicer/serializer.h>
#include <string>
#include <visibility.h>

namespace DB {
	class SelectCommand;
}

namespace Slicer {
	class DLL_PUBLIC SqlSelectDeserializer : public Slicer::Deserializer {
	public:
		explicit SqlSelectDeserializer(
				DB::SelectCommand *, std::optional<std::string> typeIdCol = std::optional<std::string>());

		void Deserialize(ModelPartForRootParam) override;

	protected:
		void DLL_PRIVATE DeserializeSimple(ModelPartParam);
		void DLL_PRIVATE DeserializeObject(ModelPartParam);
		void DLL_PRIVATE DeserializeSequence(ModelPartParam);
		void DLL_PRIVATE DeserializeRow(ModelPartParam);

		DB::SelectCommand * cmd;
		unsigned int columnCount;
		std::optional<std::string> typeIdColName;
		std::optional<unsigned int> typeIdColIdx;
	};
}

#endif
