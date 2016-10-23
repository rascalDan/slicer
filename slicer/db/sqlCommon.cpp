#include <sqlExceptions.h>
#include <boost/format.hpp>

namespace Slicer {
	void TooManyRowsReturned::ice_print(std::ostream & s) const
	{
		s << "Too many rows returned";
	}

	void NoRowsReturned::ice_print(std::ostream & s) const
	{
		s << "No rows returned";
	}

	void NoRowsFound::ice_print(std::ostream & s) const
	{
		s << "No rows found";
	}

	void UnsuitableIdFieldType::ice_print(std::ostream & s) const
	{
		static boost::format f("Unsuitable id field type [%s]");
		s << f % type;
	}

}

