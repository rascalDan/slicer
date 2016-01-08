#include "modelPartsTypes.h"

namespace Slicer {
#define MODELPARTFOR(Type, ModelPart) \
	ModelPartPtr ModelPartFor(IceUtil::Optional<Type> & t) { return new ModelPartForOptional< ModelPart<Type> >(t); } \
	ModelPartPtr ModelPartFor(IceUtil::Optional<Type> * t) { return new ModelPartForOptional< ModelPart<Type> >(t); } \
	ModelPartPtr ModelPartFor(Type & t) { return new ModelPart< Type >(t); } \
	ModelPartPtr ModelPartFor(Type * t) { return new ModelPart< Type >(t); }
	MODELPARTFOR(std::string, ModelPartForSimple);
	MODELPARTFOR(bool, ModelPartForSimple);
	MODELPARTFOR(Ice::Float, ModelPartForSimple);
	MODELPARTFOR(Ice::Double, ModelPartForSimple);
	MODELPARTFOR(Ice::Byte, ModelPartForSimple);
	MODELPARTFOR(Ice::Short, ModelPartForSimple);
	MODELPARTFOR(Ice::Int, ModelPartForSimple);
	MODELPARTFOR(Ice::Long, ModelPartForSimple);
#undef MODELPARTFOR

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

