#include "conversions.h"
#include <boost/date_time/date.hpp>
#include <boost/date_time/gregorian_calendar.hpp>
#include <boost/date_time/time.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <memory>
#include <slicer/modelParts.h>
#include <slicer/modelPartsTypes.h>
#include <stdexcept>
#include <visibility.h>

#define SHORT(x) boost::numeric_cast<::Ice::Short, int64_t>(x)
inline auto
USHORT(std::integral auto x)
{
	return boost::numeric_cast<unsigned short int>(x);
}

namespace Slicer {
	DLL_PUBLIC
	boost::posix_time::ptime
	dateTimeToPTime(const ::TestModule::DateTime & dt)
	{
		return boost::posix_time::ptime(boost::gregorian::date(USHORT(dt.year), USHORT(dt.month), USHORT(dt.day)),
				boost::posix_time::time_duration(dt.hour, dt.minute, dt.second));
	}

	DLL_PUBLIC
	::TestModule::DateTime
	ptimeToDateTime(const boost::posix_time::ptime & pt)
	{
		return ::TestModule::DateTime({SHORT(pt.date().year()), SHORT(pt.date().month()), SHORT(pt.date().day()),
				SHORT(pt.time_of_day().hours()), SHORT(pt.time_of_day().minutes()), SHORT(pt.time_of_day().seconds())});
	}

	DLL_PUBLIC
	std::string
	isoDateToString(const ::TestModule::IsoDate & in)
	{
		struct tm tm {
		};
		tm.tm_mday = in.day;
		tm.tm_mon = in.month - 1;
		tm.tm_year = in.year - 1900;
		mktime(&tm);
		std::string buf(BUFSIZ, '\0');
		auto len = strftime(buf.data(), BUFSIZ, "%Y-%m-%d", &tm);
		buf.resize(len);
		return buf;
	}

	DLL_PUBLIC
	::TestModule::IsoDate
	stringToIsoDate(const std::string & in)
	{
		struct tm tm {
		};
		auto end = strptime(in.c_str(), "%Y-%m-%d", &tm);
		if (!end || *end) {
			// LCOV_EXCL_START
			throw std::runtime_error("Invalid iso-date string: " + in);
			// LCOV_EXCL_STOP
		}
		return ::TestModule::IsoDate({SHORT(tm.tm_year + 1900), SHORT(tm.tm_mon + 1), SHORT(tm.tm_mday)});
	}

	DLL_PUBLIC
	std::string
	dateTimeToString(const ::TestModule::DateTime & in)
	{
		struct tm tm {
		};
		tm.tm_sec = in.second;
		tm.tm_min = in.minute;
		tm.tm_hour = in.hour;
		tm.tm_mday = in.day;
		tm.tm_mon = in.month - 1;
		tm.tm_year = in.year - 1900;
		tm.tm_isdst = -1;
		mktime(&tm);
		std::string buf(BUFSIZ, '\0');
		auto len = strftime(buf.data(), BUFSIZ, "%Y-%b-%d %H:%M:%S", &tm);
		buf.resize(len);
		return buf;
	}

	DLL_PUBLIC
	::TestModule::DateTime
	stringToDateTime(const std::string & in)
	{
		struct tm tm {
		};
		auto end = strptime(in.c_str(), "%Y-%b-%d %H:%M:%S", &tm);
		if (!end || *end) {
			// LCOV_EXCL_START
			throw std::runtime_error("Invalid date string: " + in);
			// LCOV_EXCL_STOP
		}
		return ::TestModule::DateTime({SHORT(tm.tm_year + 1900), SHORT(tm.tm_mon + 1), SHORT(tm.tm_mday),
				SHORT(tm.tm_hour), SHORT(tm.tm_min), SHORT(tm.tm_sec)});
	}

	DLL_PUBLIC
	Ice::optional<Ice::Int>
	str2int(const std::string & s)
	{
		if (s.empty()) {
			return IceUtil::None;
		}
		return std::stoi(s);
	}

	DLL_PUBLIC
	std::string
	int2str(const Ice::optional<Ice::Int> & i)
	{
		if (!i) {
			return std::string();
		}
		return std::to_string(*i);
	}

	DLL_PUBLIC
	std::string
	ptimeToString(const boost::posix_time::ptime & pt)
	{
		return boost::posix_time::to_iso_extended_string(pt);
	}

	DLL_PUBLIC
	boost::posix_time::ptime
	stringToPtime(const std::string & s)
	{
		return boost::posix_time::from_iso_string(s);
	}
}

namespace TestModule {
	int completions = 0;

	AbValidator::AbValidator(ClassTypePtr * m) : Slicer::ModelPartForClass<ClassType>(m) { }

	void
	AbValidator::Complete()
	{
		const auto & M = *this->Model;
		if (M->a == 0 || M->b == 0) {
			// LCOV_EXCL_START
			throw std::runtime_error("Mock error");
			// LCOV_EXCL_STOP
		}
		Slicer::ModelPartForClass<ClassType>::Complete();
		completions += 1;
	}

	MonthValidator::MonthValidator(::Ice::Short * m) : Slicer::ModelPartForSimple<::Ice::Short>(m) { }

	void
	MonthValidator::Complete()
	{
		const auto & M = *this->Model;
		if (M < 1 || M > 12) {
			// LCOV_EXCL_START
			throw std::runtime_error("This date smells fishy.");
			// LCOV_EXCL_STOP
		}
		Slicer::ModelPartForSimple<::Ice::Short>::Complete();
		completions += 1;
	}
}
namespace Slicer {
	template<>
	DLL_PUBLIC ModelPartPtr
	ModelPart::Make<TestModule::MonthValidator>(::Ice::Short * m)
	{
		return std::make_shared<TestModule::MonthValidator>(m);
	}
}
