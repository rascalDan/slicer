#include "modelPartsTypes.impl.h"

namespace Slicer {
	MODELPARTFOR(std::string, ModelPartForSimple);
	MODELPARTFOR(bool, ModelPartForSimple);
	MODELPARTFOR(Ice::Float, ModelPartForSimple);
	MODELPARTFOR(Ice::Double, ModelPartForSimple);
	MODELPARTFOR(Ice::Byte, ModelPartForSimple);
	MODELPARTFOR(Ice::Short, ModelPartForSimple);
	MODELPARTFOR(Ice::Int, ModelPartForSimple);
	MODELPARTFOR(Ice::Long, ModelPartForSimple);

	template<> std::string Slicer::ModelPartForRoot<std::string>::rootName = "String";
	template<> std::string Slicer::ModelPartForRoot<bool>::rootName = "Boolean";
	template<> std::string Slicer::ModelPartForRoot<Ice::Float>::rootName = "Float";
	template<> std::string Slicer::ModelPartForRoot<Ice::Double>::rootName = "Double";
	template<> std::string Slicer::ModelPartForRoot<Ice::Byte>::rootName = "Byte";
	template<> std::string Slicer::ModelPartForRoot<Ice::Short>::rootName = "Short";
	template<> std::string Slicer::ModelPartForRoot<Ice::Int>::rootName = "Int";
	template<> std::string Slicer::ModelPartForRoot<Ice::Long>::rootName = "Long";

	template<> std::string Slicer::ModelPartForRoot<IceUtil::Optional<std::string>>::rootName = "OptionalString";
	template<> std::string Slicer::ModelPartForRoot<IceUtil::Optional<bool>>::rootName = "OptionalBoolean";
	template<> std::string Slicer::ModelPartForRoot<IceUtil::Optional<Ice::Float>>::rootName = "OptionalFloat";
	template<> std::string Slicer::ModelPartForRoot<IceUtil::Optional<Ice::Double>>::rootName = "OptionalDouble";
	template<> std::string Slicer::ModelPartForRoot<IceUtil::Optional<Ice::Byte>>::rootName = "OptionalByte";
	template<> std::string Slicer::ModelPartForRoot<IceUtil::Optional<Ice::Short>>::rootName = "OptionalShort";
	template<> std::string Slicer::ModelPartForRoot<IceUtil::Optional<Ice::Int>>::rootName = "OptionalInt";
	template<> std::string Slicer::ModelPartForRoot<IceUtil::Optional<Ice::Long>>::rootName = "OptionalLong";

	template class Slicer::ModelPartForRoot<std::string>;
	template class Slicer::ModelPartForRoot<bool>;
	template class Slicer::ModelPartForRoot<Ice::Float>;
	template class Slicer::ModelPartForRoot<Ice::Double>;
	template class Slicer::ModelPartForRoot<Ice::Byte>;
	template class Slicer::ModelPartForRoot<Ice::Short>;
	template class Slicer::ModelPartForRoot<Ice::Int>;
	template class Slicer::ModelPartForRoot<Ice::Long>;

	template class Slicer::ModelPartForRoot<IceUtil::Optional<std::string>>;
	template class Slicer::ModelPartForRoot<IceUtil::Optional<bool>>;
	template class Slicer::ModelPartForRoot<IceUtil::Optional<Ice::Float>>;
	template class Slicer::ModelPartForRoot<IceUtil::Optional<Ice::Double>>;
	template class Slicer::ModelPartForRoot<IceUtil::Optional<Ice::Byte>>;
	template class Slicer::ModelPartForRoot<IceUtil::Optional<Ice::Short>>;
	template class Slicer::ModelPartForRoot<IceUtil::Optional<Ice::Int>>;
	template class Slicer::ModelPartForRoot<IceUtil::Optional<Ice::Long>>;

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

