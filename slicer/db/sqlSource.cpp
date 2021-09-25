#include "sqlSource.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <column.h>
#include <cstdint>

namespace Slicer {
	SqlSource::SqlSource(const DB::Column & c) : column(c) { }

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
		int64_t cb;
		column >> cb;
		b = boost::numeric_cast<Ice::Byte>(cb);
	}

	void
	SqlSource::set(Ice::Short & b) const
	{
		int64_t cb;
		column >> cb;
		b = boost::numeric_cast<Ice::Short>(cb);
	}

	void
	SqlSource::set(Ice::Int & b) const
	{
		int64_t cb;
		column >> cb;
		b = boost::numeric_cast<Ice::Int>(cb);
	}

	void
	SqlSource::set(Ice::Long & b) const
	{
		int64_t cb;
		column >> cb;
		b = boost::numeric_cast<Ice::Long>(cb);
	}

	void
	SqlSource::set(Ice::Float & b) const
	{
		double cb;
		column >> cb;
		b = boost::numeric_cast<Ice::Float>(cb);
	}

	void
	SqlSource::set(Ice::Double & b) const
	{
		column >> b;
	}

	void
	SqlSource::set(std::string & b) const
	{
		column >> b;
	}
}
