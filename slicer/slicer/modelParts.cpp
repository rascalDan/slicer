#include "modelParts.h"

namespace Slicer {
	IncorrectElementName::IncorrectElementName(const std::string & n) :
		std::invalid_argument(n)
	{
	}

	UnknownType::UnknownType(const std::string & n) :
		std::invalid_argument(n)
	{
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
}

