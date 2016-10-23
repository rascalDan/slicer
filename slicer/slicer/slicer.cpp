#include "slicer.h"
#include <common.h>
#include <boost/format.hpp>

namespace Slicer {
	Slicer::MemberChildRef::MemberChildRef(Slicer::ModelPartPtr mp, const Slicer::Metadata & md) :
		mpp(mp),
		mdr(md)
	{
	}

	ModelPartPtr
	Slicer::MemberChildRef::Child() const
	{
		return mpp;
	}

	const Metadata &
	Slicer::MemberChildRef::ChildMetaData() const
	{
		return mdr;
	}

	Slicer::ImplicitChildRef::ImplicitChildRef(ModelPartPtr m) :
		mpp(m)
	{
	}

	ModelPartPtr
	Slicer::ImplicitChildRef::Child() const
	{
		return mpp;
	}

	const Metadata &
	Slicer::ImplicitChildRef::ChildMetaData() const
	{
		return emptyMetadata;
	}

	void
	InvalidEnumerationSymbol::ice_print(std::ostream & s) const
	{
		static boost::format f("Invalid enumeration symbol [%s] for type [%s]");
		s << f % symbol % type;
	}

	void
	InvalidEnumerationValue::ice_print(std::ostream & s) const
	{
		static boost::format f("Invalid enumeration symbol [%d] for type [%s]");
		s << f % value % type;
	}

	void
	UnknownType::ice_print(std::ostream & s) const
	{
		static boost::format f("Unknown type [%s]");
		s << f % type;
	}

	void
	NoConversionFound::ice_print(std::ostream & s) const
	{
		static boost::format f("No conversion found for type [%s]");
		s << f % type;
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

	void
	IncorrectElementName::ice_print(std::ostream & s) const
	{
		static boost::format f("Incorrect element name [%s]");
		s << f % name;
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

	void
	CompilerError::ice_print(std::ostream & s) const
	{
		static boost::format f("Slicer compiler: %s");
		s << f % what;
	}
}

