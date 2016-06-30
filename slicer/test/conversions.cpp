#include "types.h"
#include <boost/numeric/conversion/cast.hpp>

#define SHORT(x) boost::numeric_cast< ::Ice::Short , int64_t >(x)

namespace Slicer {
	DLL_PUBLIC
	boost::posix_time::ptime
	dateTimeToPTime(const ::TestModule::DateTime & dt)
	{
		return boost::posix_time::ptime(boost::gregorian::date(dt.year, dt.month, dt.day),
				boost::posix_time::time_duration(dt.hour, dt.minute, dt.second));
	}

	DLL_PUBLIC
	::TestModule::DateTime
	ptimeToDateTime(const boost::posix_time::ptime & pt)
	{
		return ::TestModule::DateTime({
				SHORT(pt.date().year()), SHORT(pt.date().month()), SHORT(pt.date().day()),
				SHORT(pt.time_of_day().hours()), SHORT(pt.time_of_day().minutes()), SHORT(pt.time_of_day().seconds())
			});
	}

	DLL_PUBLIC
	std::string
	isoDateToString(const ::TestModule::IsoDate & in)
	{
		struct tm tm;
		memset(&tm, 0, sizeof(struct tm));
		tm.tm_mday = in.day;
		tm.tm_mon = in.month - 1;
		tm.tm_year = in.year - 1900;
		mktime(&tm);
		char buf[BUFSIZ];
		auto len = strftime(buf, BUFSIZ, "%Y-%m-%d", &tm);
		return std::string(buf, len);
	}

	DLL_PUBLIC
	::TestModule::IsoDate
	stringToIsoDate(const std::string & in)
	{
		struct tm tm;
		memset(&tm, 0, sizeof(struct tm));
		auto end = strptime(in.c_str(), "%Y-%m-%d", &tm);
		if (!end || *end) {
			// LCOV_EXCL_START
			throw std::runtime_error("Invalid iso-date string: " + in);
			// LCOV_EXCL_STOP
		}
		return ::TestModule::IsoDate({
				SHORT(tm.tm_year + 1900), SHORT(tm.tm_mon + 1), SHORT(tm.tm_mday)});
	}

	DLL_PUBLIC
	std::string
	dateTimeToString(const ::TestModule::DateTime & in)
	{
		struct tm tm;
		memset(&tm, 0, sizeof(struct tm));
		tm.tm_sec = in.second;
		tm.tm_min = in.minute;
		tm.tm_hour = in.hour;
		tm.tm_mday = in.day;
		tm.tm_mon = in.month- 1;
		tm.tm_year = in.year - 1900;
		mktime(&tm);
		char buf[BUFSIZ];
		auto len = strftime(buf, BUFSIZ, "%Y-%b-%d %H:%M:%S", &tm);
		return std::string(buf, len);
	}

	DLL_PUBLIC
	::TestModule::DateTime
	stringToDateTime(const std::string & in)
	{
		struct tm tm;
		memset(&tm, 0, sizeof(struct tm));
		auto end = strptime(in.c_str(), "%Y-%b-%d %H:%M:%S", &tm);
		if (!end || *end) {
			// LCOV_EXCL_START
			throw std::runtime_error("Invalid date string: " + in);
			// LCOV_EXCL_STOP
		}
		return ::TestModule::DateTime({
				SHORT(tm.tm_year + 1900), SHORT(tm.tm_mon + 1), SHORT(tm.tm_mday),
				SHORT(tm.tm_hour), SHORT(tm.tm_min), SHORT(tm.tm_sec)});
	}

	DLL_PUBLIC
	::DB::TimespanPtr
	timedurationToTimespan(const boost::posix_time::time_duration & td)
	{
		return new ::DB::Timespan(SHORT(td.hours() / 24), SHORT(td.hours() % 24), SHORT(td.minutes()), SHORT(td.seconds()));
	}

	DLL_PUBLIC
	boost::posix_time::time_duration
	timespanToTimeduration(const ::DB::TimespanPtr & ts)
	{
		return boost::posix_time::time_duration((ts->days * 24) + ts->hours, ts->minutes, ts->seconds);
	}
}

