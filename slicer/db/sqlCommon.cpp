#include <sqlExceptions.h>
#include <compileTimeFormatter.h>

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

	AdHocFormatter(UnsuitableIdFieldTypeMsg, "Unsuitable id field type [%?]");
	void UnsuitableIdFieldType::ice_print(std::ostream & s) const
	{
		UnsuitableIdFieldTypeMsg::write(s, type);
	}

}

