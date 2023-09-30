#include "modelParts.h"
#include <cxxabi.h>

namespace Slicer {
	void
	ModelPart::Create()
	{
	}

	void
	ModelPart::Complete()
	{
	}

	void
	ModelPart::OnSubclass(const ModelPartHandler &, const std::string &)
	{
		throw std::logic_error {"OnSubclass not supported on this ModelPart"};
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

	bool
	ModelPart::IsOptional() const
	{
		return false;
	}

	void
	ModelPart::OnEachChild(const ChildHandler &)
	{
	}

	bool
	ModelPart::OnAnonChild(const SubPartHandler &, const HookFilter &)
	{
		return false;
	}

	bool
	ModelPart::OnChild(const SubPartHandler &, const std::string_view, const HookFilter &, MatchCase)
	{
		return false;
	}

	void
	ModelPart::OnContained(const ModelPartHandler &)
	{
		throw std::logic_error {"OnContained not supported on this ModelPart"};
	}

	std::string
	ModelPart::demangle(const char * mangled)
	{
		auto buf = std::unique_ptr<char, decltype(free) *>(
				abi::__cxa_demangle(mangled, nullptr, nullptr, nullptr), std::free);
		return "::" + std::string(buf.get());
	}

	bool
	HookCommon::filter(const HookFilter & flt) const
	{
		return (!flt || flt(this));
	}

	void
	HookCommon::apply(const ChildHandler & ch, ModelPartParam modelPart) const
	{
		ch(*this->nameStr, modelPart, this);
	}
}
