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
	template<> ModelPartPtr ModelPart::CreateFor<Type>() { return new ModelPartType(nullptr); } \
	template<> ModelPartPtr ModelPart::CreateFor(Type & s) { return new ModelPartType(&s); } \
	template<> ModelPartPtr ModelPart::CreateFor(const Type & s) { return CreateFor(const_cast<Type &>(s)); } \
	template<> ModelPartPtr ModelPart::CreateFor(IceUtil::Optional<Type> & s) { return new ModelPartForOptional<ModelPartType>(&s); } \
	template<> ModelPartPtr ModelPart::CreateFor(const IceUtil::Optional<Type> & s) { return CreateFor(const_cast<IceUtil::Optional<Type> &>(s)); } \
	template<> ModelPartForRootPtr ModelPart::CreateRootFor(Type & s) { return new ModelPartForRoot<Type>(&s); } \
	template<> ModelPartForRootPtr ModelPart::CreateRootFor(IceUtil::Optional<Type> & s) { return new ModelPartForRoot<IceUtil::Optional<Type> >(&s); } \
	template<> ModelPartForRootPtr ModelPart::CreateRootFor(const Type & s) { return CreateRootFor(const_cast<Type &>(s)); } \
	template<> ModelPartForRootPtr ModelPart::CreateRootFor(const IceUtil::Optional<Type> & s) { return CreateRootFor(const_cast<IceUtil::Optional<Type> &>(s)); } \

#define MODELPARTFOR(Type, ModelPartType) \
	CUSTOMMODELPARTFOR(Type, ModelPartType<Type>, ModelPartType<Type>)
#define MODELPARTFORSTREAM(StreamImpl) \
	namespace Slicer { \
		template<> ModelPartForRootPtr ModelPart::CreateRootFor(const StreamImpl & stream) { \
			return new ModelPartForStreamRoot<typename StreamImpl::element_type>(const_cast<StreamImpl *>(&stream)); \
		} \
	}

namespace Slicer {
	// ModelPartForRoot
	template<typename T>
	ModelPartForRoot<T>::ModelPartForRoot(T * o) :
		ModelPartForRootBase(ModelPart::CreateFor(*o)),
		ModelObject(o)
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
	ModelPartForSimple<T>::ModelPartForSimple(T * h) :
		ModelPartModel<T>(h)
	{
	}

	template<typename T>
	void ModelPartForSimple<T>::SetValue(ValueSourcePtr s)
	{
		BOOST_ASSERT(this->Model);
		s->set(*this->Model);
	}

	template<typename T>
	void ModelPartForSimple<T>::GetValue(ValueTargetPtr s)
	{
		BOOST_ASSERT(this->Model);
		s->get(*this->Model);
	}

	// ModelPartForConverted
	template<typename T, typename M, T M::* MV>
	ModelPartForConverted<T, M, MV>::ModelPartForConverted(T * h) :
		ModelPartModel<T>(h)
	{
	}

	template<typename T, typename M, IceUtil::Optional<T> M::* MV>
	ModelPartForConverted<IceUtil::Optional<T>, M, MV>::ModelPartForConverted(IceUtil::Optional<T> * h) :
		ModelPartModel<IceUtil::Optional<T>>(h)
	{
	}

	template<typename T, typename M, IceUtil::Optional<T> M::* MV>
	bool ModelPartForConverted<IceUtil::Optional<T>, M, MV>::HasValue() const
	{
		BOOST_ASSERT(this->Model);
		return *this->Model;
	}

	// Function traits helpers
	template <typename R, typename ... Args> struct function_traits;
	template <typename R, typename ... Args> struct function_traits<std::function<R(Args...)>> {
		template<int A> struct arg {
			typedef typename std::tuple_element<A, std::tuple<Args...>>::type type;
		};
	};
	template <typename F> struct callable_traits : public function_traits<std::function<typename std::remove_pointer<F>::type>> { };

	// Converters that remove "optionalness".
	template <typename X>
	struct Coerce {
		typedef typename std::remove_const<typename std::remove_reference<X>::type>::type T;

		T & operator()(T & x) const { return x; }
		const T & operator()(const T & x) const { return x; }
		template <typename Y>
		T & operator()(IceUtil::Optional<Y> & x) const { if (!x) x = Y(); return *x; }
		template <typename Y>
		const T & operator()(const IceUtil::Optional<Y> & x) const { return *x; }
	};
	template <typename X>
	struct Coerce<IceUtil::Optional<X>> {
		typedef typename std::remove_const<typename std::remove_reference<X>::type>::type T;

		IceUtil::Optional<T> & operator()(IceUtil::Optional<T> & x) const { return x; }
		const IceUtil::Optional<T> & operator()(const IceUtil::Optional<T> & x) const { return x; }
		template <typename Y>
		IceUtil::Optional<T> operator()(Y & y) const { return y; }
	};

	// Value exists check
	template <typename X, typename Y>
	typename std::enable_if<std::is_constructible<X, Y>::value, bool>::type
	valueExists(const Y &) { return true; }
	template <typename X, typename Y>
	typename std::enable_if<std::is_constructible<X, Y>::value, bool>::type
	valueExists(const IceUtil::Optional<Y> & y) { return y; }

	template<typename ET, typename MT, typename Conv>
	inline
	bool ModelPartForConvertedBase::tryConvertFrom(const ValueSourcePtr & vsp, MT * model, const Conv & conv)
	{
		if (auto vspt = dynamic_cast<TValueSource<ET> *>(vsp.get())) {
			typedef typename callable_traits<Conv>::template arg<0>::type CA;
			ET tmp;
			vspt->set(tmp);
			auto converted = conv(Coerce<CA>()(tmp));
			if (valueExists<MT>(converted)) {
				*model = Coerce<MT>()(converted);
			}
			return true;
		}
		return false;
	}

	template<typename ET, typename MT>
	inline
	bool ModelPartForConvertedBase::tryConvertFrom(const ValueSourcePtr & vsp, MT * model)
	{
		if (auto vspt = dynamic_cast<TValueSource<ET> *>(vsp.get())) {
			if (valueExists<ET>(*model)) {
				vspt->set(Coerce<ET>()(*model));
			}
			return true;
		}
		return false;
	}

	template<typename ET, typename MT, typename Conv>
	inline
	bool ModelPartForConvertedBase::tryConvertTo(const ValueTargetPtr & vsp, const MT * model, const Conv & conv)
	{
		if (auto vspt = dynamic_cast<TValueTarget<ET> *>(vsp.get())) {
			typedef typename callable_traits<Conv>::template arg<0>::type CA;
			if (valueExists<CA>(*model)) {
				auto converted = conv(Coerce<CA>()(*model));
				if (valueExists<ET>(converted)) {
					vspt->get(Coerce<ET>()(converted));
				}
			}
			return true;
		}
		return false;
	}

	template<typename ET, typename MT>
	inline
	bool ModelPartForConvertedBase::tryConvertTo(const ValueTargetPtr & vsp, const MT * model)
	{
		if (auto vspt = dynamic_cast<TValueTarget<ET> *>(vsp.get())) {
			if (valueExists<ET>(*model)) {
				vspt->get(Coerce<ET>()(*model));
			}
			return true;
		}
		return false;
	}

	// ModelPartForOptional
	template<typename T>
	ModelPartForOptional<T>::ModelPartForOptional(IceUtil::Optional< typename T::element_type > * h) :
		ModelPartModel<IceUtil::Optional< typename T::element_type> >(h)
	{
		if (this->Model && *this->Model) {
			modelPart = new T(&**this->Model);
		}
	}

	template<typename T>
	bool ModelPartForOptional<T>::hasModel() const
	{
		BOOST_ASSERT(this->Model);
		return *this->Model;
	}

	template<typename T>
	void ModelPartForOptional<T>::Create()
	{
		BOOST_ASSERT(this->Model);
		if (!*this->Model) {
			*this->Model = typename T::element_type();
			modelPart = new T(&**this->Model);
			modelPart->Create();
		}
	}

	template<typename T>
	void ModelPartForOptional<T>::GetValue(ValueTargetPtr s)
	{
		BOOST_ASSERT(this->Model);
		if (!*this->Model) {
			*this->Model = typename T::element_type();
			modelPart = new T(&**this->Model);
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

	template<typename T>
	ModelPartForComplex<T>::HookBase::HookBase(const std::string & n) :
		HookCommon(n)
	{
	}

	template<typename T>
	const Metadata & ModelPartForComplex<T>::HookBase::GetMetadata() const
	{
		return emptyMetadata;
	}

	template<typename T>
	template<typename MT, typename MP>
	ModelPartForComplex<T>::Hook<MT, MP>::Hook(MT T::* m, const std::string & n) :
		HookBase(n),
		member(m)
	{
	}

	template<typename T>
	template<typename MT, typename MP>
	ModelPartPtr ModelPartForComplex<T>::Hook<MT, MP>::Get(T * t) const
	{
		return new MP(t ? const_cast<typename std::remove_const<MT>::type *>(&(t->*member)) : NULL);
	}

	template<typename T>
	template<typename MT, typename MP>
	ModelPartForComplex<T>::HookMetadata<MT, MP>::HookMetadata(MT T::* member, const std::string & n, const Metadata & md) :
		Hook<MT, MP>(member, n),
		metadata(md)
	{
	}

	template<typename T>
	template<typename MT, typename MP>
	const Metadata & ModelPartForComplex<T>::HookMetadata<MT, MP>::GetMetadata() const
	{
		return metadata;
	}

	// ModelPartForClass
	template<typename T>
	ModelPartForClass<T>::ModelPartForClass(element_type * h) :
			ModelPartModel<element_type>(h)
	{
	}

	template<typename T>
	void ModelPartForClass<T>::Create()
	{
		BOOST_ASSERT(this->Model);
		*this->Model = new T();
	}

	template<typename T>
	T * ModelPartForClass<T>::GetModel()
	{
		return this->Model ? this->Model->get() : nullptr;
	}

	template<typename T>
	ModelPartPtr ModelPartForClass<T>::GetSubclassModelPart(const std::string & name)
	{
		BOOST_ASSERT(this->Model);
		return ModelPartForComplexBase::getSubclassModelPart(name, this->Model);
	}

	template<typename T>
	bool ModelPartForClass<T>::HasValue() const
	{
		BOOST_ASSERT(this->Model);
		return *this->Model;
	}

	template<typename T>
	IceUtil::Optional<std::string> ModelPartForClass<T>::GetTypeIdProperty() const
	{
		return typeIdProperty;
	}

	template<typename T>
	ModelPartPtr ModelPartForClass<T>::CreateModelPart(void * p)
	{
		return new ModelPartForClass<T>(static_cast<element_type *>(p));
	}

	template<typename T>
	void ModelPartForClass<T>::deleteClassName()
	{
		delete className;
		delete typeName;
	}

	template<typename T>
	void ModelPartForClass<T>::registerClass()
	{
		initClassName();
		ModelPartForComplexBase::registerClass(*className, typeName, &ModelPartForClass<T>::CreateModelPart);
	}

	template<typename T>
	void ModelPartForClass<T>::unregisterClass()
	{
		ModelPartForComplexBase::unregisterClass(*className, typeName);
		deleteClassName();
	}

	template<typename T>
	TypeId
	ModelPartForClass<T>::GetTypeId() const
	{
		BOOST_ASSERT(this->Model);
		return ModelPartForComplexBase::GetTypeId(getTypeId(), *className);
	}

	template<typename T>
	template<typename dummy>
	const std::string & ModelPartForClass<T>::getTypeId(typename std::enable_if<std::is_base_of<Ice::Object, dummy>::value>::type *) const
	{
		return (*this->Model)->ice_id();
	}

	template<typename T>
	template<typename dummy>
	std::string ModelPartForClass<T>::getTypeId(typename std::enable_if<!std::is_base_of<Ice::Object, dummy>::value>::type *) const
	{
		return ModelPartForComplexBase::demangle(typeid(*this->Model->get()).name());
	}

	// ModelPartForStruct
	template<typename T>
	ModelPartForStruct<T>::ModelPartForStruct(T * o) :
		ModelPartModel<T>(o)
	{
	}

	template<typename T>
	T * ModelPartForStruct<T>::GetModel()
	{
		return this->Model;
	}

	template<typename T>
	bool ModelPartForStruct<T>::HasValue() const
	{
		return true;
	}

	// ModelPartForEnum
	template<typename T>
	ModelPartForEnum<T>::ModelPartForEnum(T * s) :
		ModelPartModel<T>(s)
	{
	}

	template<typename T>
	const Metadata & ModelPartForEnum<T>::GetMetadata() const
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

	template<typename T>
	void ModelPartForEnum<T>::SetValue(ValueSourcePtr s)
	{
		BOOST_ASSERT(this->Model);
		std::string val;
		s->set(val);
		*this->Model = lookup(val);
	}

	template<typename T>
	void ModelPartForEnum<T>::GetValue(ValueTargetPtr s)
	{
		BOOST_ASSERT(this->Model);
		s->get(lookup(*this->Model));
	}

	// ModelPartForSequence
	template<typename T>
	ModelPartForSequence<T>::ModelPartForSequence(T * s) :
		ModelPartModel<T>(s)
	{
	}

	template<typename T>
	void ModelPartForSequence<T>::OnEachChild(const ChildHandler & ch)
	{
		BOOST_ASSERT(this->Model);
		for(auto & element : *this->Model) {
			ch(elementName, elementModelPart(element), NULL);
		}
	}

	template<typename T>
	ChildRefPtr ModelPartForSequence<T>::GetAnonChildRef(const HookFilter &)
	{
		BOOST_ASSERT(this->Model);
		this->Model->push_back(typename element_type::value_type());
		return new ImplicitChildRef(ModelPart::CreateFor(this->Model->back()));
	}

	template<typename T>
	const Metadata & ModelPartForSequence<T>::GetMetadata() const
	{
		return metadata;
	}

	template<typename T>
	ModelPartPtr ModelPartForSequence<T>::elementModelPart(typename T::value_type & e) const
	{
		return ModelPart::CreateFor(e);
	}

	template<typename T>
	ModelPartPtr ModelPartForSequence<T>::GetContainedModelPart()
	{
		return ModelPart::CreateFor<typename T::value_type>();
	}

	// ModelPartForDictionaryElementInserter
	template<typename T>
	ModelPartForDictionaryElementInserter<T>::ModelPartForDictionaryElementInserter(T * d) :
		ModelPartForStruct<typename T::value_type>(&value),
		dictionary(d)
	{
	}

	template<typename T>
	void ModelPartForDictionaryElementInserter<T>::Complete()
	{
		dictionary->insert(value);
	}

	// ModelPartForDictionary
	template<typename T>
	ModelPartForDictionary<T>::ModelPartForDictionary(T * d) :
		ModelPartModel<T>(d)
	{
	}

	template<typename T>
	void ModelPartForDictionary<T>::OnEachChild(const ChildHandler & ch)
	{
		BOOST_ASSERT(this->Model);
		for (auto & pair : *this->Model) {
			ch(pairName, new ModelPartForStruct<typename T::value_type>(&pair), NULL);
		}
	}

	template<typename T>
	ChildRefPtr ModelPartForDictionary<T>::GetAnonChildRef(const HookFilter &)
	{
		BOOST_ASSERT(this->Model);
		return new ImplicitChildRef(new ModelPartForDictionaryElementInserter<T>(this->Model));
	}

	template<typename T>
	ChildRefPtr ModelPartForDictionary<T>::GetChildRef(const std::string & name, const HookFilter &)
	{
		BOOST_ASSERT(this->Model);
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

	template<typename T>
	ModelPartPtr ModelPartForDictionary<T>::GetContainedModelPart()
	{
		return new ModelPartForStruct<typename T::value_type>(nullptr);
	}

	// ModelPartForStream
	template<typename T>
	ModelPartForStream<T>::ModelPartForStream(Stream<T> * s) :
		ModelPartModel<Stream<T>>(s)
	{
	}

	template<typename T>
	ModelPartPtr
	ModelPartForStream<T>::GetContainedModelPart()
	{
		return ModelPart::CreateFor<T>();
	}

	template<typename T>
	void
	ModelPartForStream<T>::OnEachChild(const ChildHandler & ch)
	{
		BOOST_ASSERT(this->Model);
		this->Model->Produce([&ch](const T & element) {
			ch(ModelPartForSequence<std::vector<T>>::elementName, ModelPart::CreateFor(element), NULL);
		});
	}

	template<typename T>
	ModelPartForStreamRoot<T>::ModelPartForStreamRoot(Stream<T> * s) :
		ModelPartForStreamRootBase(new ModelPartForStream<T>(s))
	{
	}

	template<typename T>
	const std::string &
	ModelPartForStreamRoot<T>::GetRootName() const
	{
		return ModelPartForRoot<std::vector<T>>::rootName;
	}
}

#endif

