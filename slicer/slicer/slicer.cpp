#include "slicer.h"
#include <common.h>
#include <compileTimeFormatter.h>

namespace Slicer {
	Slicer::ChildRef::ChildRef() :
		mpp(),
		mdr(emptyMetadata)
	{
	}

	Slicer::ChildRef::ChildRef(ModelPartPtr m) :
		mpp(m),
		mdr(emptyMetadata)
	{
	}

	Slicer::ChildRef::ChildRef(Slicer::ModelPartPtr mp, const Slicer::Metadata & md) :
		mpp(mp),
		mdr(md)
	{
	}

	ModelPartPtr
	Slicer::ChildRef::Child() const
	{
		return mpp;
	}

	Slicer::ChildRef::operator bool() const
	{
		return mpp;
	}

	const Metadata &
	Slicer::ChildRef::ChildMetaData() const
	{
		return mdr;
	}

	AdHocFormatter(InvalidEnumerationSymbolMsg, "Invalid enumeration symbol [%?] for type [%?]");
	void
	InvalidEnumerationSymbol::ice_print(std::ostream & s) const
	{
		InvalidEnumerationSymbolMsg::write(s, symbol, type);
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
}

