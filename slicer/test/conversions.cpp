#include "types.h"
#include <boost/numeric/conversion/cast.hpp>

#define SHORT(x) boost::numeric_cast< ::Ice::Short >(x)

namespace Slicer {
	boost::posix_time::ptime
	dateTimeToPTime(const ::TestModule::DateTime &)
	{
		throw std::runtime_error("Not implemented");
	}

	::TestModule::DateTime
	ptimeToDateTime(const boost::posix_time::ptime &)
	{
		throw std::runtime_error("Not implemented");
	}

	std::string
	dateTimeToString(const ::TestModule::DateTime & in)
	{
		char buf[BUFSIZ];
		struct tm tm({ in.second, in.minute, in.hour, in.day, in.month, in.year, 0, 0, 0
#ifdef _BSD_SOURCE
				, 0, 0
#endif
				});
		mktime(&tm);
		auto len = strftime(buf, BUFSIZ, "%Y-%b-%d %H:%M:%S", &tm);
		return std::string(buf, len);
	}

	::TestModule::DateTime
	stringToDateTime(const std::string & in)
	{
		struct tm tm;
		memset(&tm, 0, sizeof(struct tm));
		auto end = strptime(in.c_str(), "%Y-%b-%d %H:%M:%S", &tm);
		if (!end || *end) {
			throw std::runtime_error("Invalid date string: " + in);
		}
		return ::TestModule::DateTime({
				SHORT(tm.tm_year), SHORT(tm.tm_mon), SHORT(tm.tm_mday),
				SHORT(tm.tm_hour), SHORT(tm.tm_min), SHORT(tm.tm_sec)});
	}
}

