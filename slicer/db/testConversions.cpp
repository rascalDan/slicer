#include <Ice/Config.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <cstdint>
#include <memory>
#include <testModels.h>
#include <visibility.h>

#define SHORT(x) boost::numeric_cast<::Ice::Short, int64_t>(x)

namespace Slicer {
	DLL_PUBLIC
	::TestDatabase::TimespanPtr
	timedurationToTimespan(const boost::posix_time::time_duration & td)
	{
		return std::make_shared<::TestDatabase::Timespan>(
				SHORT(td.hours() / 24), SHORT(td.hours() % 24), SHORT(td.minutes()), SHORT(td.seconds()));
	}

	DLL_PUBLIC
	boost::posix_time::time_duration
	timespanToTimeduration(const ::TestDatabase::TimespanPtr & ts)
	{
		return boost::posix_time::time_duration {(ts->days * 24) + ts->hours, ts->minutes, ts->seconds};
	}
}
