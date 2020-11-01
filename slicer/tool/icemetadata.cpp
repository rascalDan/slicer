#include "icemetadata.h"
#include <boost/algorithm/string/predicate.hpp>

namespace Slicer {
	IceMetaData::IceMetaData(Slice::StringList && a) : arr {std::forward<Slice::StringList>(a)}
	{
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
		return std::count_if(_begin, _end, isSlicerMetaData);
	}

	bool
	IceMetaData::hasSlicerMetaData() const
	{
		return std::any_of(_begin, _end, isSlicerMetaData);
	}
}
