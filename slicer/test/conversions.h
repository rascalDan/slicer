#ifndef SLICER_TEST_CONVERSIONS_H
#define SLICER_TEST_CONVERSIONS_H

#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace TestModule {
	class DateTime;
}
namespace Slicer {
	::TestModule::DateTime
	stringToDateTime(const std::string & in);
	std::string
	dateTimeToString(const ::TestModule::DateTime & in);
}

#endif

