#pragma once

#include <optional>
#include <slicer/modelParts.h>
#include <slicer/serializer.h>
#include <string>
#include <visibility.h>

namespace DB {
	class SelectCommand;
	class Column;
}

namespace Slicer {
	class DLL_PUBLIC SqlSelectDeserializer : public Slicer::Deserializer {
	public:
		explicit SqlSelectDeserializer(
				DB::SelectCommand *, std::optional<std::string> typeIdCol = std::optional<std::string>());

		void Deserialize(ModelPartForRootParam) override;

	protected:
		DLL_PRIVATE void DeserializeSimple(ModelPartParam);
		DLL_PRIVATE void DeserializeObject(ModelPartParam);
		DLL_PRIVATE void DeserializeSequence(ModelPartParam);
		DLL_PRIVATE void DeserializeRow(ModelPartParam);
		DLL_PRIVATE void fillLowerColumnNameCache();
		DLL_PRIVATE inline const DB::Column * searchOrFilleColumnCache(size_t idx, const HookCommon * hook);

		DB::SelectCommand * cmd;
		unsigned int columnCount;
		std::optional<std::string> typeIdColName;
		std::optional<unsigned int> typeIdColIdx;
		std::vector<std::string> lowerColumnNames;
		std::vector<const DB::Column *> orderedColumns;
	};
}
