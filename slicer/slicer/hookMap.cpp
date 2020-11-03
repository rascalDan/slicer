#include "hookMap.h"
#include <boost/algorithm/string/case_conv.hpp>

namespace Slicer {
	void
	to_lower(std::string s)
	{
		boost::algorithm::to_lower(s);
	}
}
