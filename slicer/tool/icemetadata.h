#ifndef SLICER_TOOL_METADATA_H
#define SLICER_TOOL_METADATA_H

#include <Slice/Parser.h>
#include <c++11Helpers.h>
#include <list>
#include <metadata.h>

namespace Slicer {
	class DLL_PUBLIC IceMetaData : public MetaData<false, std::string> {
	public:
		static constexpr std::string_view slicer_prefix {"slicer"};

		explicit IceMetaData() = default;
		explicit IceMetaData(Slice::StringList && arr);
		~IceMetaData() = default;

		SPECIAL_MEMBERS_DELETE(IceMetaData);

		[[nodiscard]] static std::vector<std::string_view> split(std::string_view metaData);

		[[nodiscard]] bool hasSlicerMetaData() const;
		[[nodiscard]] size_t countSlicerMetaData() const;

		[[nodiscard]] constexpr bool static isSlicerMetaData(PairView md)
		{
			return in_scope(md.first, slicer_prefix);
		}

		[[nodiscard]] auto
		getSlicerMetaData() const
		{
			return values(slicer_prefix);
		}

	private:
		ContainerBase<> arr;
	};
}

#endif
