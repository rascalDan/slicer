#include "modelParts.h"
#include <boost/lexical_cast.hpp>

namespace Slicer {
	const Metadata emptyMetadata;

	IncorrectElementName::IncorrectElementName(const std::string & n) :
		std::invalid_argument(n)
	{
	}

	UnknownType::UnknownType(const std::string & n) :
		std::invalid_argument(n)
	{
	}
	InvalidEnumerationValue::InvalidEnumerationValue(const std::string & n, const std::string & e) :
		std::invalid_argument("No such value '" + n + "' in " + e) { }

	InvalidEnumerationValue::InvalidEnumerationValue(::Ice::Int n, const std::string & e) :
		std::invalid_argument("Invalid value " + boost::lexical_cast<std::string>(n) + " in " + e) { }

	ClassNameMap * &
	classNameMap()
	{
		static ClassNameMap * refs = new ClassNameMap();
		return refs;
	}

	const std::string &
	ModelPart::ToModelTypeName(const std::string & name)
	{
		auto mapped = classNameMap()->right.find(name);
		if (mapped != classNameMap()->right.end()) {
			return mapped->second;
		}
		return name;
	}

	const std::string &
	ModelPart::ToExchangeTypeName(const std::string & name)
	{
		auto mapped = classNameMap()->left.find(name);
		if (mapped != classNameMap()->left.end()) {
			return mapped->second;
		}
		return name;
	}

	ClassRefMap * &
	classRefMap()
	{
		static ClassRefMap * refs = new ClassRefMap();
		return refs;
	}

	void
	ModelPart::Create()
	{
	}

	void
	ModelPart::Complete()
	{
	}

	ModelPartPtr
	ModelPart::GetSubclassModelPart(const std::string &)
	{
		return this;
	}

	TypeId
	ModelPart::GetTypeId() const
	{
		return TypeId();
	}

	IceUtil::Optional<std::string>
	ModelPart::GetTypeIdProperty() const
	{
		return IceUtil::Optional<std::string>();
	}

	void
	ModelPart::SetValue(ValueSourcePtr)
	{
	}

	void
	ModelPart::GetValue(ValueTargetPtr)
	{
	}

	const Metadata &
	ModelPart::GetMetadata() const
	{
		return emptyMetadata;
	}

	ModelPartPtr
	ModelPart::GetAnonChild(const HookFilter & flt)
	{
		auto ref = GetAnonChildRef(flt);
		return ref ? ref->Child() : ModelPartPtr(NULL);
	}

	ModelPartPtr
	ModelPart::GetChild(const std::string & memberName, const HookFilter & flt)
	{
		auto ref = GetChildRef(memberName, flt);
		return ref ? ref->Child() : ModelPartPtr(NULL);
	}

	template<> std::string Slicer::ModelPartForRoot<std::string>::rootName = "String";
	template<> std::string Slicer::ModelPartForRoot<bool>::rootName = "Boolean";
	template<> std::string Slicer::ModelPartForRoot<Ice::Float>::rootName = "Float";
	template<> std::string Slicer::ModelPartForRoot<Ice::Double>::rootName = "Double";
	template<> std::string Slicer::ModelPartForRoot<Ice::Byte>::rootName = "Byte";
	template<> std::string Slicer::ModelPartForRoot<Ice::Short>::rootName = "Short";
	template<> std::string Slicer::ModelPartForRoot<Ice::Int>::rootName = "Int";
	template<> std::string Slicer::ModelPartForRoot<Ice::Long>::rootName = "Long";
}

