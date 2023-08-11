#include "sqlSource.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <column.h>
#include <cstdint>

namespace Slicer {
	SqlSource::SqlSource(const DB::Column & c) : column(c) { }

	namespace {
		template<typename C, typename T>
		void
		numericSet(const DB::Column & column, T & b)
		{
			C cb {};
			column >> cb;
			b = boost::numeric_cast<T>(cb);
		}
	}

	bool
	SqlSource::isNull() const
	{
		return column.isNull();
	}

	void
	SqlSource::set(boost::posix_time::ptime & b) const
	{
		column >> b;
	}

	void
	SqlSource::set(boost::posix_time::time_duration & b) const
	{
		column >> b;
	}

	void
	SqlSource::set(bool & b) const
	{
		column >> b;
	}

	void
	SqlSource::set(Ice::Byte & b) const
	{
		numericSet<int64_t>(column, b);
	}

	void
	SqlSource::set(Ice::Short & b) const
	{
		numericSet<int64_t>(column, b);
	}

	void
	SqlSource::set(Ice::Int & b) const
	{
		numericSet<int64_t>(column, b);
	}

	void
	SqlSource::set(Ice::Long & b) const
	{
		numericSet<int64_t>(column, b);
	}

	void
	SqlSource::set(Ice::Float & b) const
	{
		numericSet<double>(column, b);
	}

	void
	SqlSource::set(Ice::Double & b) const
	{
		numericSet<double>(column, b);
	}

	void
	SqlSource::set(std::string & b) const
	{
		column >> b;
	}
}
