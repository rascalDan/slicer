#include "slicer.h"
#include "slicer/modelParts.h"
#include <common.h>
#include <compileTimeFormatter.h>
#include <utility>

namespace Slicer {
	AdHocFormatter(InvalidEnumerationSymbolMsg, "Invalid enumeration symbol [%?] for type [%?]");

	void
	InvalidEnumerationSymbol::ice_print(std::ostream & s) const
	{
		InvalidEnumerationSymbolMsg::write(s, symbol, type);
	}

	AdHocFormatter(IncorrectTypeMsg, "Type [%?] cannot be used as a [%?]");

	void
	IncorrectType::ice_print(std::ostream & s) const
	{
		IncorrectTypeMsg::write(s, type, target);
	}

	AdHocFormatter(InvalidEnumerationValueMsg, "Invalid enumeration symbol [%?] for type [%?]");

	void
	InvalidEnumerationValue::ice_print(std::ostream & s) const
	{
		InvalidEnumerationValueMsg::write(s, value, type);
	}

	AdHocFormatter(UnknownTypeMsg, "Unknown type [%?]");

	void
	UnknownType::ice_print(std::ostream & s) const
	{
		UnknownTypeMsg::write(s, type);
	}

	AdHocFormatter(NoConversionFoundMsg, "No conversion found for type [%?]");

	void
	NoConversionFound::ice_print(std::ostream & s) const
	{
		NoConversionFoundMsg::write(s, type);
	}

	void
	LocalTypeException::ice_print(std::ostream & s) const
	{
		s << "Invalid operation on local type";
	}

	void
	UnsupportedModelType::ice_print(std::ostream & s) const
	{
		s << "Unsupported model type";
	}

	AdHocFormatter(IncorrectElementNameMsg, "Incorrect element name [%?]");

	void
	IncorrectElementName::ice_print(std::ostream & s) const
	{
		IncorrectElementNameMsg::write(s, name);
	}

	void
	DeserializerError::ice_print(std::ostream & s) const
	{
		s << "General deserializer error";
	}

	void
	SerializerError::ice_print(std::ostream & s) const
	{
		s << "General serializer error";
	}

	void
	RuntimeError::ice_print(std::ostream & s) const
	{
		s << "General runtime error";
	}

	AdHocFormatter(CompilerErrorMsg, "Slicer compiler: %?");

	void
	CompilerError::ice_print(std::ostream & s) const
	{
		CompilerErrorMsg::write(s, what);
	}

	AdHocFormatter(InvalidStreamOperationMsg, "%? is not valid on streams");

	void
	InvalidStreamOperation::ice_print(std::ostream & s) const
	{
		InvalidStreamOperationMsg::write(s, method);
	}

	AdHocFormatter(AbstractClassExceptionMsg, "%? is an abstract class");

	void
	AbstractClassException::ice_print(std::ostream & s) const
	{
		AbstractClassExceptionMsg::write(s, type);
	}
}
