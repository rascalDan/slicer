#ifndef SLICER_MODELPARTSTYPES_IMPL_H
#define SLICER_MODELPARTSTYPES_IMPL_H

#include "modelPartsTypes.h"
#include "common.h"
#include <Ice/Stream.h>
#include <Ice/StreamHelpers.h>
#include <Ice/BasicStream.h>
#include <IceUtil/Optional.h>

#define CUSTOMMODELPARTFOR(Type, BaseModelPart, ModelPartType) \
	template class BaseModelPart; \
	template class ModelPartForRoot<Type>; \
	template class ModelPartForRoot< IceUtil::Optional<Type> >; \
	template<> ModelPartPtr ModelPart::CreateFor(Type & s) { return new ModelPartType(s); } \
	template<> ModelPartPtr ModelPart::CreateFor(IceUtil::Optional<Type> & s) { return new ModelPartForOptional<ModelPartType>(s); } \
	template<> ModelPartForRootPtr ModelPart::CreateRootFor(Type & s) { return new ModelPartForRoot<Type>(s); } \
	template<> ModelPartForRootPtr ModelPart::CreateRootFor(IceUtil::Optional<Type> & s) { return new ModelPartForRoot<IceUtil::Optional<Type> >(s); } \

#define MODELPARTFOR(Type, ModelPartType) \
	CUSTOMMODELPARTFOR(Type, ModelPartType<Type>, ModelPartType<Type>)

namespace Slicer {
	// ModelPartForRoot
	template<typename T>
	ModelPartForRoot<T>::ModelPartForRoot(T & o) :
		ModelPartForRootBase(ModelPart::CreateFor(o)),
		ModelObject(&o)
	{
	}

	template<typename T>
	const std::string & ModelPartForRoot<T>::GetRootName() const
	{
		return rootName;
	}

	template<typename T>
	bool ModelPartForRoot<T>::HasValue() const
	{
		return ModelObject && mp->HasValue();
	}

#define IfLocal(T) \
	typename std::enable_if<Slicer::isLocal<T>::value>::type
#define IfNotLocal(T) \
	typename std::enable_if<!Slicer::isLocal<T>::value>::type

	template<typename T>
	IfNotLocal(T)
	typeWrite(::Ice::OutputStreamPtr & s, const ::IceUtil::Optional<T> & m)
	{
		typedef Ice::StreamableTraits<T> traits;
		typedef Ice::StreamOptionalHelper<T, traits::helper, traits::fixedLength> SOH;
		s->startEncapsulation();
		if (m && s->writeOptional(0, SOH::optionalFormat)) {
			SOH::write(s.get(), *m);
		}
		s->endEncapsulation();
	}

	template<typename T>
	IfLocal(T)
	typeWrite(::Ice::OutputStreamPtr &, const ::IceUtil::Optional<T> &)
	{
		throw LocalTypeException();
	}

	template<typename T>
	IfNotLocal(T)
	typeWrite(::Ice::OutputStreamPtr & s, const T & m)
	{
		s->write(m);
	}

	template<typename T>
	IfLocal(T)
	typeWrite(::Ice::OutputStreamPtr &, const T &)
	{
		throw LocalTypeException();
	}

	template<typename T>
	IfNotLocal(T)
	typeRead(::Ice::InputStreamPtr & s, ::IceUtil::Optional<T> & m)
	{
		typedef Ice::StreamableTraits<T> traits;
		typedef Ice::StreamOptionalHelper<T, traits::helper, traits::fixedLength> SOH;
		s->startEncapsulation();
		if (s->readOptional(0, SOH::optionalFormat)) {
			m.__setIsSet();
			SOH::read(s.get(), *m);
		}
		else {
			m = IceUtil::None;
		}
		s->endEncapsulation();
	}

	template<typename T>
	IfLocal(T)
	typeRead(::Ice::InputStreamPtr &, ::IceUtil::Optional<T> &)
	{
		throw LocalTypeException();
	}

	template<typename T>
	IfNotLocal(T)
	typeRead(::Ice::InputStreamPtr & s, T & m)
	{
		s->read(m);
	}

	template<typename T>
	IfLocal(T)
	typeRead(::Ice::InputStreamPtr &, T &)
	{
		throw LocalTypeException();
	}

	template<typename T>
	void ModelPartForRoot<T>::Write(::Ice::OutputStreamPtr & s) const
	{
		typeWrite(s, *ModelObject);
	}

	template<typename T>
	void ModelPartForRoot<T>::Read(::Ice::InputStreamPtr & s)
	{
		typeRead(s, *ModelObject);
	}

	// ModelPartForSimple
	template<typename T>
	ModelPartForSimple<T>::ModelPartForSimple(T & h) :
		ModelPartModel<T>(h)
	{
	}

	template<typename T>
	void ModelPartForSimple<T>::SetValue(ValueSourcePtr s)
	{
		s->set(this->Model);
	}

	template<typename T>
	void ModelPartForSimple<T>::GetValue(ValueTargetPtr s)
	{
		s->get(this->Model);
	}

	// ModelPartForConverted
	template<typename T, typename M, T M::* MV>
	ModelPartForConverted<T, M, MV>::ModelPartForConverted(T & h) :
		ModelPartModel<T>(h)
	{
	}

	// ModelPartForOptional
	template<typename T>
	ModelPartForOptional<T>::ModelPartForOptional(IceUtil::Optional< typename T::element_type > & h) :
		ModelPartModel<IceUtil::Optional< typename T::element_type> >(h)
	{
		if (this->Model) {
			modelPart = new T(*this->Model);
		}
	}

	template<typename T>
	bool ModelPartForOptional<T>::hasModel() const
	{
		return this->Model;
	}

	template<typename T>
	void ModelPartForOptional<T>::Create()
	{
		if (!this->Model) {
			this->Model = typename T::element_type();
			modelPart = new T(*this->Model);
			modelPart->Create();
		}
	}

	template<typename T>
	void ModelPartForOptional<T>::GetValue(ValueTargetPtr s)
	{
		if (!this->Model) {
			this->Model = typename T::element_type();
			modelPart = new T(*this->Model);
		}
		modelPart->GetValue(s);
	}

	template<typename T>
	ModelPartType ModelPartForOptional<T>::GetType() const
	{
		return T::type;
	}

	// ModelPartForComplex
	template<typename T>
	void ModelPartForComplex<T>::OnEachChild(const ChildHandler & ch)
	{
		for (const auto & h : hooks) {
			h->apply(ch, h->Get(GetModel()));
		}
	}

	template<typename T>
	ChildRefPtr ModelPartForComplex<T>::GetAnonChildRef(const HookFilter & flt)
	{
		for (const auto & h : hooks) {
			if (h->filter(flt)) {
				return new MemberChildRef(h->Get(GetModel()), h->GetMetadata());
			}
		}
		return NULL;
	}

	template<typename T>
	ChildRefPtr ModelPartForComplex<T>::GetChildRef(const std::string & name, const HookFilter & flt)
	{
		for (const auto & h : hooks) {
			if (h->filter(flt, name)) {
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
	ModelPartForClass<T>::ModelPartForClass(element_type & h) :
			ModelPartModel<element_type>(h)
	{
	}

	template<typename T>
	void ModelPartForClass<T>::Create()
	{
		this->Model = new T();
	}

	template<typename T>
	T * ModelPartForClass<T>::GetModel()
	{
		return this->Model.get();
	}

	template<typename T>
	ModelPartPtr ModelPartForClass<T>::GetSubclassModelPart(const std::string & name)
	{
		return ModelPartForComplexBase::getSubclassModelPart(name, &this->Model);
	}

	template<typename T>
	bool ModelPartForClass<T>::HasValue() const
	{
		return this->Model;
	}

	template<typename T>
	IceUtil::Optional<std::string> ModelPartForClass<T>::GetTypeIdProperty() const
	{
		return typeIdProperty;
	}

	template<typename T>
	ModelPartPtr ModelPartForClass<T>::CreateModelPart(void * p)
	{
		return new ModelPartForClass<T>(*static_cast<element_type *>(p));
	}

	template<typename T>
	void ModelPartForClass<T>::registerClass()
	{
		ModelPartForComplexBase::registerClass(className, typeName, &ModelPartForClass<T>::CreateModelPart);
	}

	template<typename T>
	void ModelPartForClass<T>::unregisterClass()
	{
		ModelPartForComplexBase::unregisterClass(className, typeName);
	}

	template<typename T>
	TypeId
	ModelPartForClass<T>::GetTypeId() const
	{
		return ModelPartForComplexBase::GetTypeId(this->Model->ice_id(), className);
	}

	// ModelPartForStruct
	template<typename T>
	ModelPartForStruct<T>::ModelPartForStruct(T & o) :
		ModelPartModel<T>(o)
	{
	}

	template<typename T>
	T * ModelPartForStruct<T>::GetModel()
	{
		return &this->Model;
	}

	template<typename T>
	bool ModelPartForStruct<T>::HasValue() const
	{
		return true;
	}

	// ModelPartForEnum
	template<typename T>
	ModelPartForEnum<T>::ModelPartForEnum(T & s) :
		ModelPartModel<T>(s)
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
		ModelPartModel<T>(s)
	{
	}

	template<typename T>
	void ModelPartForSequence<T>::OnEachChild(const ChildHandler & ch)
		{
			for(auto & element : this->Model) {
				ch(elementName, elementModelPart(element), NULL);
			}
		}

	template<typename T>
	ChildRefPtr ModelPartForSequence<T>::GetAnonChildRef(const HookFilter &)
	{
		this->Model.push_back(typename element_type::value_type());
		return new ImplicitChildRef(ModelPart::CreateFor(this->Model.back()));
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
			throw InvalidEnumerationSymbol(val, typeid(T).name());
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
		ModelPartModel<T>(d)
	{
	}

	template<typename T>
	void ModelPartForDictionary<T>::OnEachChild(const ChildHandler & ch)
	{
		for (auto & pair : this->Model) {
			ch(pairName, new ModelPartForStruct<typename T::value_type>(pair), NULL);
		}
	}

	template<typename T>
	ChildRefPtr ModelPartForDictionary<T>::GetAnonChildRef(const HookFilter &)
	{
		return new ImplicitChildRef(new ModelPartForDictionaryElementInserter<T>(this->Model));
	}

	template<typename T>
	ChildRefPtr ModelPartForDictionary<T>::GetChildRef(const std::string & name, const HookFilter &)
	{
		if (name != pairName) {
			throw IncorrectElementName(name);
		}
		return new ImplicitChildRef(new ModelPartForDictionaryElementInserter<T>(this->Model));
	}

	template<typename T>
	const Metadata & ModelPartForDictionary<T>::GetMetadata() const
	{
		return metadata;
	}

}

#endif

