#include "modelParts.h"

namespace Slicer {
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
		return shared_from_this();
	}

	TypeId
	ModelPart::GetTypeId() const
	{
		return {};
	}

	std::optional<std::string>
	ModelPart::GetTypeIdProperty() const
	{
		return {};
	}

	void
	ModelPart::SetValue(ValueSource &&)
	{
	}

	bool
	ModelPart::GetValue(ValueTarget &&)
	{
		return false;
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
		return ref ? ref.Child() : ModelPartPtr(nullptr);
	}

	ModelPartPtr
	ModelPart::GetChild(std::string_view memberName, const HookFilter & flt)
	{
		auto ref = GetChildRef(memberName, flt);
		return ref ? ref.Child() : ModelPartPtr(nullptr);
	}

	bool
	ModelPart::IsOptional() const
	{
		return false;
	}

	ModelPartPtr
	ModelPart::GetContainedModelPart()
	{
		return shared_from_this();
	}

	bool
	HookCommon::filter(const HookFilter & flt) const
	{
		return (!flt || flt(this));
	}

	void
	HookCommon::apply(const ChildHandler & ch, const ModelPartPtr & modelPart) const
	{
		ch(*this->nameStr, modelPart, this);
	}
}
