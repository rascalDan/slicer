#include "modelParts.h"
#include <boost/lexical_cast.hpp>

namespace Slicer {
	const Metadata emptyMetadata;
	static void createClassMaps() __attribute__((constructor(208)));
	static void deleteClassMaps() __attribute__((destructor(208)));
	static ClassNameMap * names;
	static ClassRefMap * refs;

	void createClassMaps()
	{
		names = new ClassNameMap();
		refs = new ClassRefMap();
	}

	static void deleteClassMaps()
	{
		delete names;
		delete refs;
		names = nullptr;
		refs = nullptr;
	}

	ClassNameMap *
	classNameMap()
	{
		return names;
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

	ClassRefMap *
	classRefMap()
	{
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

	bool
	ModelPart::GetValue(ValueTargetPtr)
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
		return ref ? ref->Child() : ModelPartPtr(NULL);
	}

	ModelPartPtr
	ModelPart::GetChild(const std::string & memberName, const HookFilter & flt)
	{
		auto ref = GetChildRef(memberName, flt);
		return ref ? ref->Child() : ModelPartPtr(NULL);
	}

	bool
	ModelPart::IsOptional() const
	{
		return false;
	}

	ModelPartPtr
	ModelPart::GetContainedModelPart()
	{
		return this;
	}

	HookCommon::HookCommon(const std::string & n) :
		name(n)
	{
	}

	bool
	HookCommon::filter(const HookFilter & flt, const std::string & name)
	{
		return (this->name == name && (!flt || flt(this)));
	}

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

