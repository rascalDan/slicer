#include "modelParts.h"

namespace Slicer {
	const Metadata emptyMetadata;
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
		return TypeId();
	}

	Ice::optional<std::string>
	ModelPart::GetTypeIdProperty() const
	{
		return Ice::optional<std::string>();
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
	ModelPart::GetChild(const std::string & memberName, const HookFilter & flt)
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

	HookCommon::HookCommon(std::string n) : name(std::move(n)) { }

	bool
	HookCommon::filter(const HookFilter & flt)
	{
		return (!flt || flt(this));
	}

	void
	HookCommon::apply(const ChildHandler & ch, const ModelPartPtr & modelPart)
	{
		ch(this->name, modelPart, this);
	}
}
