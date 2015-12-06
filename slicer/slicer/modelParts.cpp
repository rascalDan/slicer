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

	template<> DLL_PUBLIC std::string Slicer::ModelPartForRoot<std::string>::rootName = "String";
	template<> DLL_PUBLIC std::string Slicer::ModelPartForRoot<bool>::rootName = "Boolean";
	template<> DLL_PUBLIC std::string Slicer::ModelPartForRoot<Ice::Float>::rootName = "Float";
	template<> DLL_PUBLIC std::string Slicer::ModelPartForRoot<Ice::Double>::rootName = "Double";
	template<> DLL_PUBLIC std::string Slicer::ModelPartForRoot<Ice::Byte>::rootName = "Byte";
	template<> DLL_PUBLIC std::string Slicer::ModelPartForRoot<Ice::Short>::rootName = "Short";
	template<> DLL_PUBLIC std::string Slicer::ModelPartForRoot<Ice::Int>::rootName = "Int";
	template<> DLL_PUBLIC std::string Slicer::ModelPartForRoot<Ice::Long>::rootName = "Long";

	template<> DLL_PUBLIC std::string Slicer::ModelPartForRoot<IceUtil::Optional<std::string>>::rootName = "OptionalString";
	template<> DLL_PUBLIC std::string Slicer::ModelPartForRoot<IceUtil::Optional<bool>>::rootName = "OptionalBoolean";
	template<> DLL_PUBLIC std::string Slicer::ModelPartForRoot<IceUtil::Optional<Ice::Float>>::rootName = "OptionalFloat";
	template<> DLL_PUBLIC std::string Slicer::ModelPartForRoot<IceUtil::Optional<Ice::Double>>::rootName = "OptionalDouble";
	template<> DLL_PUBLIC std::string Slicer::ModelPartForRoot<IceUtil::Optional<Ice::Byte>>::rootName = "OptionalByte";
	template<> DLL_PUBLIC std::string Slicer::ModelPartForRoot<IceUtil::Optional<Ice::Short>>::rootName = "OptionalShort";
	template<> DLL_PUBLIC std::string Slicer::ModelPartForRoot<IceUtil::Optional<Ice::Int>>::rootName = "OptionalInt";
	template<> DLL_PUBLIC std::string Slicer::ModelPartForRoot<IceUtil::Optional<Ice::Long>>::rootName = "OptionalLong";

	void ModelPartForSimpleBase::OnEachChild(const ChildHandler &) { }
	ChildRefPtr ModelPartForSimpleBase::GetAnonChildRef(const HookFilter &) { return NULL; }
	ChildRefPtr ModelPartForSimpleBase::GetChildRef(const std::string &, const HookFilter &) { return NULL; }
	bool ModelPartForSimpleBase::HasValue() const { return true; }
	ModelPartType ModelPartForSimpleBase::GetType() const { return type; }
	ModelPartType ModelPartForSimpleBase::type = mpt_Simple;

	void ModelPartForConvertedBase::OnEachChild(const ChildHandler &) { }
	ChildRefPtr ModelPartForConvertedBase::GetAnonChildRef(const HookFilter &) { return NULL; }
	ChildRefPtr ModelPartForConvertedBase::GetChildRef(const std::string &, const HookFilter &) { return NULL; }
	bool ModelPartForConvertedBase::HasValue() const { return true; }
	ModelPartType ModelPartForConvertedBase::GetType() const { return type; }
	ModelPartType ModelPartForConvertedBase::type = mpt_Simple;

	ModelPartType ModelPartForComplexBase::GetType() const { return type; }
	ModelPartType ModelPartForComplexBase::type = mpt_Complex;

	void ModelPartForEnumBase::OnEachChild(const ChildHandler &) { }
	ChildRefPtr ModelPartForEnumBase::GetAnonChildRef(const HookFilter &) { return NULL; }
	ChildRefPtr ModelPartForEnumBase::GetChildRef(const std::string &, const HookFilter &) { return NULL; }
	bool ModelPartForEnumBase::HasValue() const { return true; }
	ModelPartType ModelPartForEnumBase::GetType() const { return type; }
	ModelPartType ModelPartForEnumBase::type = mpt_Simple;

	bool ModelPartForSequenceBase::HasValue() const { return true; }
	ModelPartType ModelPartForSequenceBase::GetType() const { return type; }
	ModelPartType ModelPartForSequenceBase::type = mpt_Sequence;

	bool ModelPartForDictionaryBase::HasValue() const { return true; }
	ModelPartType ModelPartForDictionaryBase::GetType() const { return type; }
	ModelPartType ModelPartForDictionaryBase::type = mpt_Dictionary;
}

