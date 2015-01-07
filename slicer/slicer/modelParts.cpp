#include "modelParts.h"

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

	template<> std::string Slicer::ModelPartForRoot<std::string>::rootName = "String";
	template<> std::string Slicer::ModelPartForRoot<bool>::rootName = "Boolean";
	template<> std::string Slicer::ModelPartForRoot<Ice::Float>::rootName = "Float";
	template<> std::string Slicer::ModelPartForRoot<Ice::Double>::rootName = "Double";
	template<> std::string Slicer::ModelPartForRoot<Ice::Byte>::rootName = "Byte";
	template<> std::string Slicer::ModelPartForRoot<Ice::Short>::rootName = "Short";
	template<> std::string Slicer::ModelPartForRoot<Ice::Int>::rootName = "Int";
	template<> std::string Slicer::ModelPartForRoot<Ice::Long>::rootName = "Long";
}

