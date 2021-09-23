#include "icemetadata.h"
#include <boost/algorithm/string/predicate.hpp>

namespace Slicer {
	IceMetaData::IceMetaData(Slice::StringList && a)
	{
		arr.reserve(a.size());
		std::for_each(a.begin(), a.end(), [this](auto && a) {
			auto & md = arr.emplace_back(a, std::string_view {});
			md.second = std::string_view(md.first).substr(0, md.first.rfind(':'));
		});
		_begin = arr.begin();
		_end = arr.end();
	}

	std::vector<std::string_view>
	IceMetaData::split(std::string_view metaData)
	{
		std::vector<std::string_view> output;

		for (size_t first = 0; first < metaData.size();) {
			const auto second = metaData.find(':', first);

			if (first != second) {
				output.emplace_back(metaData.substr(first, second - first));
			}

			if (second == std::string_view::npos) {
				break;
			}

			first = second + 1;
		}

		return output;
	}

	size_t
	IceMetaData::countSlicerMetaData() const
	{
		return static_cast<size_t>(std::count_if(_begin, _end, isSlicerMetaData));
	}

	bool
	IceMetaData::hasSlicerMetaData() const
	{
		return std::any_of(_begin, _end, isSlicerMetaData);
	}
}
