#include "hookMap.h"
#include <boost/algorithm/string/case_conv.hpp>

namespace Slicer {
	void
	to_lower(std::string & s)
	{
		boost::algorithm::to_lower(s);
	}

	std::string
	to_lower_copy(const std::string & s)
	{
		return boost::algorithm::to_lower_copy(s);
	}
}
