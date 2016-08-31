#ifndef SLICER_MODELPARTSTYPES_IMPL_H
#define SLICER_MODELPARTSTYPES_IMPL_H

#include "modelPartsTypes.h"

#define MODELPARTFOR(Type, ModelPartType) \
	template<> ModelPartPtr ModelPart::CreateFor(Type & s) { return new ModelPartType<Type>(s); } \
	template<> ModelPartPtr ModelPart::CreateFor(IceUtil::Optional<Type> & s) { return new ModelPartForOptional<ModelPartType<Type> >(s); } \
	template<> ModelPartPtr ModelPart::CreateRootFor(Type & s) { return new ModelPartForRoot<Type>(s); } \
	template<> ModelPartPtr ModelPart::CreateRootFor(IceUtil::Optional<Type> & s) { return new ModelPartForRoot<IceUtil::Optional<Type> >(s); } \

namespace Slicer {
	// ModelPartForRoot
	template<typename T>
	ModelPartForRoot<T>::ModelPartForRoot(T & o) :
		ModelObject(&o),
		mp(ModelPart::CreateFor(o))
	{
	}

	template<typename T>
	ChildRefPtr ModelPartForRoot<T>::GetAnonChildRef(const HookFilter &)
	{
		mp->Create();
		return new ImplicitChildRef(mp);
	}

	template<typename T>
	ChildRefPtr ModelPartForRoot<T>::GetChildRef(const std::string & name, const HookFilter &)
	{
		if (name != rootName) {
			throw IncorrectElementName(name);
		}
		mp->Create();
		return new ImplicitChildRef(mp);
	}

	template<typename T>
	void ModelPartForRoot<T>::OnEachChild(const ChildHandler & ch)
	{
		ch(rootName, mp, NULL);
	}

	template<typename T>
	bool ModelPartForRoot<T>::HasValue() const
	{
		return ModelObject && mp->HasValue();
	}

	template<typename T>
	ModelPartType ModelPartForRoot<T>::GetType() const
	{
		return mp->GetType();
	}

	template<typename T>
	bool ModelPartForRoot<T>::IsOptional() const
	{
		return mp->IsOptional();
	}

	// ModelPartForSimple
	template<typename T>
	ModelPartForSimple<T>::ModelPartForSimple(T & h) :
		Member(h)
	{
	}

	template<typename T>
	void ModelPartForSimple<T>::SetValue(ValueSourcePtr s)
	{
		s->set(Member);
	}

	template<typename T>
	void ModelPartForSimple<T>::GetValue(ValueTargetPtr s)
	{
		s->get(Member);
	}

	// ModelPartForConverted
	template<typename T, typename M, T M::* MV>
	ModelPartForConverted<T, M, MV>::ModelPartForConverted(T & h) :
		Member(h)
	{
	}

	// ModelPartForOptional
	template<typename T>
	ModelPartForOptional<T>::ModelPartForOptional(IceUtil::Optional< typename T::element_type > & h) :
		OptionalMember(h)
	{
		if (OptionalMember) {
			modelPart = new T(*OptionalMember);
		}
	}

	template<typename T>
	void ModelPartForOptional<T>::OnEachChild(const ChildHandler & ch)
	{
		if (OptionalMember) {
			modelPart->OnEachChild(ch);
		}
	}

	template<typename T>
	void ModelPartForOptional<T>::Complete()
	{
		if (OptionalMember) {
			modelPart->Complete();
		}
	}

	template<typename T>
	void ModelPartForOptional<T>::Create()
	{
		if (!OptionalMember) {
			OptionalMember = typename T::element_type();
			modelPart = new T(*OptionalMember);
			modelPart->Create();
		}
	}

	template<typename T>
	ChildRefPtr ModelPartForOptional<T>::GetAnonChildRef(const HookFilter & flt)
	{
		if (OptionalMember) {
			return modelPart->GetAnonChildRef(flt);
		}
		return NULL;
	}

	template<typename T>
	ChildRefPtr ModelPartForOptional<T>::GetChildRef(const std::string & name, const HookFilter & flt)
	{
		if (OptionalMember) {
			return modelPart->GetChildRef(name, flt);
		}
		return NULL;
	}

	template<typename T>
	void ModelPartForOptional<T>::SetValue(ValueSourcePtr s)
	{
		if (OptionalMember) {
			modelPart->SetValue(s);
		}
	}

	template<typename T>
	void ModelPartForOptional<T>::GetValue(ValueTargetPtr s)
	{
		if (!OptionalMember) {
			OptionalMember = typename T::element_type();
			modelPart = new T(*OptionalMember);
		}
		modelPart->GetValue(s);
	}

	template<typename T>
	bool ModelPartForOptional<T>::HasValue() const
	{
		return OptionalMember && modelPart->HasValue();
	}

	template<typename T>
	ModelPartType ModelPartForOptional<T>::GetType() const
	{
		return T::type;
	}

	template<typename T>
	bool ModelPartForOptional<T>::IsOptional() const
	{
		return true;
	};

	template<typename T>
	const Metadata & ModelPartForOptional<T>::GetMetadata() const
	{
		return modelPart->GetMetadata();
	}

	// ModelPartForComplex
	template<typename T>
	void ModelPartForComplex<T>::OnEachChild(const ChildHandler & ch)
	{
		for (const auto & h : hooks) {
			auto modelPart = h->Get(GetModel());
			ch(h->PartName(), modelPart && modelPart->HasValue() ? modelPart : ModelPartPtr(), h);
		}
	}

	template<typename T>
	ChildRefPtr ModelPartForComplex<T>::GetAnonChildRef(const HookFilter & flt)
	{
		for (const auto & h : hooks) {
			if (!flt || flt(h)) {
				return new MemberChildRef(h->Get(GetModel()), h->GetMetadata());
			}
		}
		return NULL;
	}

	template<typename T>
	ChildRefPtr ModelPartForComplex<T>::GetChildRef(const std::string & name, const HookFilter & flt)
	{
		for (const auto & h : hooks) {
			if (h->PartName() == name && (!flt || flt(h))) {
				return new MemberChildRef(h->Get(GetModel()), h->GetMetadata());
			}
		}
		return NULL;
	}

	template<typename T>
	const Metadata & ModelPartForComplex<T>::GetMetadata() const
	{
		return metadata;
	}

	// ModelPartForClass
	template<typename T>
	ModelPartForClass<T>::ModelPartForClass(T & h) :
			ModelObject(h)
	{
	}

	template<typename T>
	void ModelPartForClass<T>::Create()
	{
		ModelObject = new typename T::element_type();
	}

	template<typename T>
	typename T::element_type * ModelPartForClass<T>::GetModel()
	{
		return ModelObject.get();
	}

	template<typename T>
	ModelPartPtr ModelPartForClass<T>::GetSubclassModelPart(const std::string & name)
	{
		auto ref = classRefMap()->find(ModelPart::ToModelTypeName(name));
		if (ref == classRefMap()->end()) {
			throw UnknownType(name);
		}
		return ref->second(&this->ModelObject);
	}

	template<typename T>
	bool ModelPartForClass<T>::HasValue() const
	{
		return ModelObject;
	}

	template<typename T>
	IceUtil::Optional<std::string> ModelPartForClass<T>::GetTypeIdProperty() const
	{
		return typeIdProperty;
	}

	// ModelPartForStruct
	template<typename T>
	ModelPartForStruct<T>::ModelPartForStruct(T & o) :
		ModelObject(o)
	{
	}

	template<typename T>
	T * ModelPartForStruct<T>::GetModel()
	{
		return &ModelObject;
	}

	template<typename T>
	bool ModelPartForStruct<T>::HasValue() const
	{
		return true;
	}

	// ModelPartForEnum
	template<typename T>
	ModelPartForEnum<T>::ModelPartForEnum(T & s) :
		modelPart(s)
	{
	}

	template<typename T>
	const Metadata & ModelPartForEnum<T>::GetMetadata() const
	{
		return metadata;
	}

	// ModelPartForSequence
	template<typename T>
	ModelPartForSequence<T>::ModelPartForSequence(T & s) :
		sequence(s)
	{
	}

	template<typename T>
	void ModelPartForSequence<T>::OnEachChild(const ChildHandler & ch)
		{
			for(auto & element : sequence) {
				ch(elementName, elementModelPart(element), NULL);
			}
		}

	template<typename T>
	ChildRefPtr ModelPartForSequence<T>::GetAnonChildRef(const HookFilter &)
	{
		sequence.push_back(typename element_type::value_type());
		return new ImplicitChildRef(ModelPart::CreateFor(sequence.back()));
	}

	template<typename T>
	const Metadata & ModelPartForSequence<T>::GetMetadata() const
	{
		return metadata;
	}

	template<typename T>
	T ModelPartForEnum<T>::lookup(const std::string & val)
	{
		auto i = enumerations.right.find(val);
		if (i == enumerations.right.end()) {
			throw InvalidEnumerationValue(val, typeid(T).name());
		}
		return i->second;
	}

	template<typename T>
	const std::string & ModelPartForEnum<T>::lookup(T val)
	{
		auto i = enumerations.left.find(val);
		if (i == enumerations.left.end()) {
			throw InvalidEnumerationValue((::Ice::Int)val, typeid(T).name());
		}
		return i->second;
	}

	// ModelPartForDictionaryElementInserter
	template<typename T>
	ModelPartForDictionaryElementInserter<T>::ModelPartForDictionaryElementInserter(T & d) :
		ModelPartForStruct<typename T::value_type>(value),
		dictionary(d)
	{
	}

	template<typename T>
	void ModelPartForDictionaryElementInserter<T>::Complete()
	{
		dictionary.insert(value);
	}

	// ModelPartForDictionary
	template<typename T>
	ModelPartForDictionary<T>::ModelPartForDictionary(T & d) :
		dictionary(d)
	{
	}

	template<typename T>
	void ModelPartForDictionary<T>::OnEachChild(const ChildHandler & ch)
	{
		for (auto & pair : dictionary) {
			ch(pairName, new ModelPartForStruct<typename T::value_type>(pair), NULL);
		}
	}

	template<typename T>
	ChildRefPtr ModelPartForDictionary<T>::GetAnonChildRef(const HookFilter &)
	{
		return new ImplicitChildRef(new ModelPartForDictionaryElementInserter<T>(dictionary));
	}

	template<typename T>
	ChildRefPtr ModelPartForDictionary<T>::GetChildRef(const std::string & name, const HookFilter &)
	{
		if (name != pairName) {
			throw IncorrectElementName(name);
		}
		return new ImplicitChildRef(new ModelPartForDictionaryElementInserter<T>(dictionary));
	}

	template<typename T>
	const Metadata & ModelPartForDictionary<T>::GetMetadata() const
	{
		return metadata;
	}

}

#endif

