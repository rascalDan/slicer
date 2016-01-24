#ifndef SLICER_TEST_CONVERSIONS_H
#define SLICER_TEST_CONVERSIONS_H

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <visibility.h>

namespace TestModule {
	class DateTime;
}
namespace Slicer {
	DLL_PUBLIC
	::TestModule::DateTime
	stringToDateTime(const std::string & in);
	DLL_PUBLIC
	std::string
	dateTimeToString(const ::TestModule::DateTime & in);
}

#endif

