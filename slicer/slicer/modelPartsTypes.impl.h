#ifndef SLICER_MODELPARTSTYPES_IMPL_H
#define SLICER_MODELPARTSTYPES_IMPL_H

#include "common.h"
#include "modelPartsTypes.h"
#include <Ice/StreamHelpers.h>
#include <IceUtil/Optional.h>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/multi_index/global_fun.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index_container.hpp>
#include <c++11Helpers.h>

#define CUSTOMMODELPARTFOR(Type, BaseModelPart, ModelPartType) \
	template<> DLL_PUBLIC ModelPartPtr ModelPart::CreateFor<Type>() \
	{ \
		return std::make_shared<ModelPartType>(nullptr); \
	} \
	template<> DLL_PUBLIC ModelPartPtr ModelPart::CreateFor(Type & s) \
	{ \
		return std::make_shared<ModelPartType>(&s); \
	} \
	template<> DLL_PUBLIC ModelPartPtr ModelPart::CreateFor(const Type & s) \
	{ \
		return CreateFor(const_cast<Type &>(s)); \
	} \
	template<> DLL_PUBLIC ModelPartPtr ModelPart::CreateFor(Ice::optional<Type> & s) \
	{ \
		return std::make_shared<ModelPartForOptional<ModelPartType>>(&s); \
	} \
	template<> DLL_PUBLIC ModelPartPtr ModelPart::CreateFor(const Ice::optional<Type> & s) \
	{ \
		return CreateFor(const_cast<Ice::optional<Type> &>(s)); \
	} \
	template<> DLL_PUBLIC ModelPartForRootPtr ModelPart::CreateRootFor(Type & s) \
	{ \
		return std::make_shared<ModelPartForRoot<Type>>(&s); \
	} \
	template<> DLL_PUBLIC ModelPartForRootPtr ModelPart::CreateRootFor(Ice::optional<Type> & s) \
	{ \
		return std::make_shared<ModelPartForRoot<Ice::optional<Type>>>(&s); \
	} \
	template<> DLL_PUBLIC ModelPartForRootPtr ModelPart::CreateRootFor(const Type & s) \
	{ \
		return CreateRootFor(const_cast<Type &>(s)); \
	} \
	template<> DLL_PUBLIC ModelPartForRootPtr ModelPart::CreateRootFor(const Ice::optional<Type> & s) \
	{ \
		return CreateRootFor(const_cast<Ice::optional<Type> &>(s)); \
	} \
	template class BaseModelPart; \
	template class ModelPartForRoot<Type>; \
	template class ModelPartForRoot<Ice::optional<Type>>;

#define MODELPARTFOR(Type, ModelPartType) CUSTOMMODELPARTFOR(Type, ModelPartType<Type>, ModelPartType<Type>)
#define MODELPARTFORSTREAM(StreamImpl) \
	namespace Slicer { \
		template<> \
		DLL_PUBLIC ModelPartForRootPtr \
		ModelPart::CreateRootFor(const StreamImpl & stream) \
		{ \
			return std::make_shared<ModelPartForStreamRoot<typename StreamImpl::element_type>>( \
					const_cast<StreamImpl *>(&stream)); \
		} \
	}

#ifdef __clang__
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wundefined-var-template"
#endif
namespace Slicer {
	// ModelPartForRoot
	template<typename T>
	ModelPartForRoot<T>::ModelPartForRoot(T * o) : ModelPartForRootBase(ModelPart::CreateFor(*o)), ModelObject(o)
	{
	}

	template<typename T>
	const std::string &
	ModelPartForRoot<T>::GetRootName() const
	{
		return rootName;
	}

	template<typename T>
	bool
	ModelPartForRoot<T>::HasValue() const
	{
		return ModelObject && mp->HasValue();
	}

	template<typename T>
	void
	typeWrite(::Ice::OutputStream & s, const ::Ice::optional<T> & m)
	{
		if constexpr (!isLocal<T>::value) {
			using traits = Ice::StreamableTraits<T>;
			using SOH = Ice::StreamOptionalHelper<T, traits::helper, traits::fixedLength>;
			s.startEncapsulation();
			if (m && s.writeOptional(0, SOH::optionalFormat)) {
				SOH::write(&s, *m);
			}
			s.endEncapsulation();
		}
		else {
			throw LocalTypeException();
		}
	}

	template<typename T>
	void
	typeWrite(::Ice::OutputStream & s, const T & m)
	{
		if constexpr (!isLocal<T>::value) {
			s.write(m);
		}
		else {
			throw LocalTypeException();
		}
	}

	template<typename T>
	void
	typeRead(::Ice::InputStream & s, ::Ice::optional<T> & m)
	{
		if constexpr (!isLocal<T>::value) {
			using traits = Ice::StreamableTraits<T>;
			using SOH = Ice::StreamOptionalHelper<T, traits::helper, traits::fixedLength>;
			s.startEncapsulation();
			if (s.readOptional(0, SOH::optionalFormat)) {
				m = T();
				SOH::read(&s, *m);
			}
			else {
				m = IceUtil::None;
			}
			s.endEncapsulation();
		}
		else {
			throw LocalTypeException();
		}
	}

	template<typename T>
	void
	typeRead(::Ice::InputStream & s, T & m)
	{
		if constexpr (!isLocal<T>::value) {
			s.read(m);
		}
		else {
			throw LocalTypeException();
		}
	}

	template<typename T>
	void
	ModelPartForRoot<T>::Write(::Ice::OutputStream & s) const
	{
		typeWrite(s, *ModelObject);
	}

	template<typename T>
	void
	ModelPartForRoot<T>::Read(::Ice::InputStream & s)
	{
		typeRead(s, *ModelObject);
	}

	// ModelPartForSimple
	template<typename T> ModelPartForSimple<T>::ModelPartForSimple(T * h) : ModelPartModel<T>(h) { }

	template<typename T>
	void
	ModelPartForSimple<T>::SetValue(ValueSource && s)
	{
		BOOST_ASSERT(this->Model);
		s.set(*this->Model);
	}

	template<typename T>
	bool
	ModelPartForSimple<T>::GetValue(ValueTarget && s)
	{
		BOOST_ASSERT(this->Model);
		s.get(*this->Model);
		return true;
	}

	// ModelPartForConverted
	template<typename T, typename M, T M::*MV>
	ModelPartForConverted<T, M, MV>::ModelPartForConverted(T * h) : ModelPartModel<T>(h)
	{
	}

	template<typename T, typename M, Ice::optional<T> M::*MV>
	ModelPartForConverted<Ice::optional<T>, M, MV>::ModelPartForConverted(Ice::optional<T> * h) :
		ModelPartModel<Ice::optional<T>>(h)
	{
	}

	template<typename T, typename M, Ice::optional<T> M::*MV>
	bool
	ModelPartForConverted<Ice::optional<T>, M, MV>::HasValue() const
	{
		BOOST_ASSERT(this->Model);
		return (bool)*this->Model;
	}

	// Function traits helpers
	template<typename R, typename... Args> struct function_traits;
	template<typename R, typename... Args> struct function_traits<std::function<R(Args...)>> {
		template<int A> struct arg {
			using type = typename std::tuple_element<A, std::tuple<Args...>>::type;
		};
	};
	template<typename F>
	struct callable_traits : public function_traits<std::function<typename std::remove_pointer<F>::type>> {
	};

	// Converters that remove "optionalness".
	template<typename X> struct Coerce {
		using T = typename std::remove_const<typename std::remove_reference<X>::type>::type;

		T &
		operator()(T & x) const
		{
			return x;
		}
		const T &
		operator()(const T & x) const
		{
			return x;
		}
		template<typename Y>
		T &
		operator()(Ice::optional<Y> & x) const
		{
			if (!x) {
				x = Y();
			}
			return *x;
		}
		template<typename Y>
		const T &
		operator()(const Ice::optional<Y> & x) const
		{
			return *x;
		}
		static bool
		valueExists(const T &)
		{
			return true;
		}
		static bool
		valueExists(const Ice::optional<T> & y)
		{
			return y.has_value();
		}
	};
	template<typename X> struct Coerce<Ice::optional<X>> {
		using T = typename std::remove_const<typename std::remove_reference<X>::type>::type;

		Ice::optional<T> &
		operator()(Ice::optional<T> & x) const
		{
			return x;
		}
		const Ice::optional<T> &
		operator()(const Ice::optional<T> & x) const
		{
			return x;
		}
		template<typename Y>
		Ice::optional<T>
		operator()(Y & y) const
		{
			return y;
		}
		static bool
		valueExists(const T &)
		{
			return true;
		}
		static bool
		valueExists(const Ice::optional<T> &)
		{
			return true;
		}
	};

	template<typename ET, typename MT, typename Conv>
	inline bool
	ModelPartForConvertedBase::tryConvertFrom(ValueSource & vsp, MT * model, const Conv & conv)
	{
		if (auto vspt = dynamic_cast<TValueSource<ET> *>(&vsp)) {
			using CA = typename callable_traits<Conv>::template arg<0>::type;
			ET tmp;
			vspt->set(tmp);
			auto converted = conv(Coerce<CA>()(tmp));
			if (Coerce<MT>::valueExists(converted)) {
				*model = Coerce<MT>()(converted);
			}
			return true;
		}
		return false;
	}

	template<typename ET, typename MT>
	inline bool
	ModelPartForConvertedBase::tryConvertFrom(ValueSource & vsp, MT * model)
	{
		if (auto vspt = dynamic_cast<TValueSource<ET> *>(&vsp)) {
			if (Coerce<ET>::valueExists(*model)) {
				vspt->set(Coerce<ET>()(*model));
			}
			return true;
		}
		return false;
	}

	template<typename ET, typename MT, typename Conv>
	inline TryConvertResult
	ModelPartForConvertedBase::tryConvertTo(ValueTarget & vsp, const MT * model, const Conv & conv)
	{
		if (auto vspt = dynamic_cast<TValueTarget<ET> *>(&vsp)) {
			using CA = typename callable_traits<Conv>::template arg<0>::type;
			using CAR = typename std::remove_const<typename std::remove_reference<CA>::type>::type;
			if (Coerce<CAR>::valueExists(*model)) {
				auto converted = conv(Coerce<CA>()(*model));
				if (Coerce<ET>::valueExists(converted)) {
					vspt->get(Coerce<ET>()(converted));
					return TryConvertResult::Value;
				}
			}
			return TryConvertResult::NoValue;
		}
		return TryConvertResult::NoAction;
	}

	template<typename ET, typename MT>
	inline TryConvertResult
	ModelPartForConvertedBase::tryConvertTo(ValueTarget & vsp, const MT * model)
	{
		if (auto vspt = dynamic_cast<TValueTarget<ET> *>(&vsp)) {
			if (Coerce<ET>::valueExists(*model)) {
				vspt->get(Coerce<ET>()(*model));
				return TryConvertResult::Value;
			}
			return TryConvertResult::NoValue;
		}
		return TryConvertResult::NoAction;
	}

	// ModelPartForOptional
	template<typename T>
	ModelPartForOptional<T>::ModelPartForOptional(Ice::optional<typename T::element_type> * h) :
		ModelPartModel<Ice::optional<typename T::element_type>>(h)
	{
		if (this->Model && *this->Model) {
			modelPart = std::make_shared<T>(&**this->Model);
		}
	}

	template<typename T>
	bool
	ModelPartForOptional<T>::hasModel() const
	{
		BOOST_ASSERT(this->Model);
		return (bool)*this->Model;
	}

	template<typename T>
	void
	ModelPartForOptional<T>::Create()
	{
		BOOST_ASSERT(this->Model);
		if (!*this->Model) {
			*this->Model = typename T::element_type();
			modelPart = std::make_shared<T>(&**this->Model);
			modelPart->Create();
		}
	}

	template<typename T>
	bool
	ModelPartForOptional<T>::GetValue(ValueTarget && s)
	{
		BOOST_ASSERT(this->Model);
		if (*this->Model) {
			return modelPart->GetValue(std::move(s));
		}
		return false;
	}

	template<typename T>
	ModelPartType
	ModelPartForOptional<T>::GetType() const
	{
		return T::type;
	}

	// ModelPartForComplex
	template<typename T>
	class ModelPartForComplex<T>::Hooks :
		public boost::multi_index_container<HookPtr,
				boost::multi_index::indexed_by<boost::multi_index::sequenced<>,
						boost::multi_index::ordered_non_unique<
								boost::multi_index::member<HookCommon, const std::string, &HookCommon::name>,
								std::less<>>,
						boost::multi_index::ordered_non_unique<
								boost::multi_index::member<HookCommon, const std::string, &HookCommon::name>,
								case_less>>> {
	};

	template<typename T>
	void
	ModelPartForComplex<T>::OnEachChild(const ChildHandler & ch)
	{
		for (const auto & h : hooks) {
			h->apply(ch, h->Get(GetModel()));
		}
	}

	template<typename P>
	auto
	begin(const P & p)
	{
		return p.first;
	}
	template<typename P>
	auto
	end(const P & p)
	{
		return p.second;
	}
	template<typename T>
	template<typename R>
	ChildRef
	ModelPartForComplex<T>::GetChildRefFromRange(const R & range, const HookFilter & flt)
	{
		const auto itr = std::find_if(boost::begin(range), boost::end(range), [&flt](auto && h) {
			return h->filter(flt);
		});
		if (itr != boost::end(range)) {
			const auto & h = *itr;
			auto model = GetModel();
			return ChildRef(h->Get(model), h->GetMetadata());
		}
		return ChildRef();
	}

	template<typename T>
	ChildRef
	ModelPartForComplex<T>::GetAnonChildRef(const HookFilter & flt)
	{
		return GetChildRefFromRange(hooks.template get<0>(), flt);
	}

	template<typename T>
	ChildRef
	ModelPartForComplex<T>::GetChildRef(const std::string & name, const HookFilter & flt, bool matchCase)
	{
		if (matchCase) {
			return GetChildRefFromRange(hooks.template get<1>().equal_range(name), flt);
		}
		else {
			return GetChildRefFromRange(hooks.template get<2>().equal_range(name), flt);
		}
	}

	template<typename T>
	const Metadata &
	ModelPartForComplex<T>::GetMetadata() const
	{
		return metadata;
	}

	template<typename T>
	template<typename H, typename... P>
	void
	ModelPartForComplex<T>::addHook(Hooks & h, const P &... p)
	{
		h.push_back(std::make_unique<H>(p...));
	}

	template<typename T> class DLL_PRIVATE ModelPartForComplex<T>::HookBase : public HookCommon {
	public:
		explicit HookBase(const std::string & n) : HookCommon(n) { }
		SPECIAL_MEMBERS_DEFAULT(HookBase);
		virtual ~HookBase() = default;

		virtual ModelPartPtr Get(T * t) const = 0;
		[[nodiscard]] const Metadata &
		GetMetadata() const override
		{
			return emptyMetadata;
		}
	};

	template<typename T>
	template<typename MT, typename MP>
	class DLL_PRIVATE ModelPartForComplex<T>::Hook : public ModelPartForComplex<T>::HookBase {
	public:
		Hook(MT T::*m, const std::string & n) : HookBase(n), member(m) { }

		ModelPartPtr
		Get(T * t) const override
		{
			return std::make_shared<MP>(
					t ? const_cast<typename std::remove_const<MT>::type *>(&(t->*member)) : nullptr);
		}

	private:
		const MT T::*member;
	};

	template<typename T>
	template<typename MT, typename MP>
	class DLL_PRIVATE ModelPartForComplex<T>::HookMetadata : public ModelPartForComplex<T>::template Hook<MT, MP> {
	public:
		HookMetadata(MT T::*member, const std::string & n, Metadata md) :
			Hook<MT, MP>(member, n), hookMetadata(std::move(md))
		{
		}

		[[nodiscard]] const Metadata &
		GetMetadata() const override
		{
			return hookMetadata;
		}

	private:
		const Metadata hookMetadata;
	};

	// ModelPartForClass
	template<typename T> ModelPartForClass<T>::ModelPartForClass(element_type * h) : ModelPartModel<element_type>(h) { }

	template<typename T>
	void
	ModelPartForClass<T>::Create()
	{
		BOOST_ASSERT(this->Model);
		*this->Model = std::make_shared<T>();
	}

	template<typename T>
	T *
	ModelPartForClass<T>::GetModel()
	{
		return this->Model ? this->Model->get() : nullptr;
	}

	template<typename T>
	ModelPartPtr
	ModelPartForClass<T>::GetSubclassModelPart(const std::string & name)
	{
		BOOST_ASSERT(this->Model);
		return ModelPartForComplexBase::getSubclassModelPart(name, this->Model);
	}

	template<typename T>
	bool
	ModelPartForClass<T>::HasValue() const
	{
		BOOST_ASSERT(this->Model);
		return (bool)*this->Model;
	}

	template<typename T>
	Ice::optional<std::string>
	ModelPartForClass<T>::GetTypeIdProperty() const
	{
		return typeIdProperty;
	}

	template<typename T>
	ModelPartPtr
	ModelPartForClass<T>::CreateModelPart(void * p)
	{
		return std::make_shared<ModelPartForClass<T>>(static_cast<element_type *>(p));
	}

	template<typename T>
	void
	ModelPartForClass<T>::deleteClassName()
	{
		delete className;
		delete typeName;
	}

	template<typename T>
	void
	ModelPartForClass<T>::registerClass()
	{
		initClassName();
		ModelPartForComplexBase::registerClass(*className, typeName, &ModelPartForClass<T>::CreateModelPart);
	}

	template<typename T>
	void
	ModelPartForClass<T>::unregisterClass()
	{
		BOOST_ASSERT(className);
		ModelPartForComplexBase::unregisterClass(*className, typeName);
		deleteClassName();
	}

	template<typename T>
	TypeId
	ModelPartForClass<T>::GetTypeId() const
	{
		BOOST_ASSERT(this->Model);
		BOOST_ASSERT(className);
		return ModelPartForComplexBase::GetTypeId(getTypeId(), *className);
	}

	template<typename T>
	template<typename dummy>
	const std::string &
	ModelPartForClass<T>::getTypeId(typename std::enable_if<std::is_base_of<Ice::Object, dummy>::value>::type *) const
	{
		BOOST_ASSERT(this->Model);
		return (*this->Model)->ice_id();
	}

	template<typename T>
	template<typename dummy>
	std::string
	ModelPartForClass<T>::getTypeId(typename std::enable_if<!std::is_base_of<Ice::Object, dummy>::value>::type *) const
	{
		BOOST_ASSERT(this->Model);
		return ModelPartForComplexBase::demangle(typeid(*this->Model->get()).name());
	}

	// ModelPartForStruct
	template<typename T> ModelPartForStruct<T>::ModelPartForStruct(T * o) : ModelPartModel<T>(o) { }

	template<typename T>
	T *
	ModelPartForStruct<T>::GetModel()
	{
		return this->Model;
	}

	template<typename T>
	bool
	ModelPartForStruct<T>::HasValue() const
	{
		return true;
	}

	// ModelPartForEnum
	template<typename T> using EnumPair = std::pair<T, std::string>;

	template<typename T>
	class ModelPartForEnum<T>::Enumerations :
		public boost::multi_index_container<EnumPair<T>,
				boost::multi_index::indexed_by<boost::multi_index::ordered_unique<boost::multi_index::member<
													   EnumPair<T>, const T, &EnumPair<T>::first>>,
						boost::multi_index::ordered_unique<
								boost::multi_index::member<EnumPair<T>, const std::string, &EnumPair<T>::second>,
								std::less<>>>> {
	};

	template<typename T> ModelPartForEnum<T>::ModelPartForEnum(T * s) : ModelPartModel<T>(s) { }

	template<typename T>
	const Metadata &
	ModelPartForEnum<T>::GetMetadata() const
	{
		return metadata;
	}

	template<int Side, typename Ex, typename ExP, typename T, typename V, typename R>
	inline const auto &
	ModelPartForEnumLookup(
			const typename ModelPartForEnum<T>::Enumerations & enumerations, const V & val, R EnumPair<T>::*rv)
	{
		const auto & side = enumerations.template get<Side>();
		if (auto i = side.find(val); i != side.end()) {
			return (*i).*rv;
		}
		throw Ex(ExP(val), typeid(T).name());
	}

	template<typename T>
	T
	ModelPartForEnum<T>::lookup(const std::string_view & val)
	{
		return ModelPartForEnumLookup<1, InvalidEnumerationSymbol, std::string, T>(
				enumerations, val, &EnumPair<T>::first);
	}

	template<typename T>
	const std::string &
	ModelPartForEnum<T>::lookup(T val)
	{
		return ModelPartForEnumLookup<0, InvalidEnumerationValue, ::Ice::Int, T>(
				enumerations, val, &EnumPair<T>::second);
	}

	template<typename T>
	void
	ModelPartForEnum<T>::SetValue(ValueSource && s)
	{
		BOOST_ASSERT(this->Model);
		std::string val;
		s.set(val);
		*this->Model = lookup(val);
	}

	template<typename T>
	bool
	ModelPartForEnum<T>::GetValue(ValueTarget && s)
	{
		BOOST_ASSERT(this->Model);
		s.get(lookup(*this->Model));
		return true;
	}

	// ModelPartForSequence
	template<typename T> ModelPartForSequence<T>::ModelPartForSequence(T * s) : ModelPartModel<T>(s) { }

	template<typename T>
	void
	ModelPartForSequence<T>::OnEachChild(const ChildHandler & ch)
	{
		BOOST_ASSERT(this->Model);
		for (auto & element : *this->Model) {
			ch(elementName, elementModelPart(element), NULL);
		}
	}

	template<typename T>
	ChildRef
	ModelPartForSequence<T>::GetAnonChildRef(const HookFilter &)
	{
		BOOST_ASSERT(this->Model);
		this->Model->push_back(typename element_type::value_type());
		return ChildRef(ModelPart::CreateFor(this->Model->back()));
	}

	template<typename T>
	const Metadata &
	ModelPartForSequence<T>::GetMetadata() const
	{
		return metadata;
	}

	template<typename T>
	ModelPartPtr
	ModelPartForSequence<T>::elementModelPart(typename T::value_type & e) const
	{
		return ModelPart::CreateFor(e);
	}

	template<typename T>
	ModelPartPtr
	ModelPartForSequence<T>::GetContainedModelPart()
	{
		return ModelPart::CreateFor<typename T::value_type>();
	}

	// ModelPartForDictionaryElementInserter
	template<typename T>
	ModelPartForDictionaryElementInserter<T>::ModelPartForDictionaryElementInserter(T * d) :
		ModelPartForStruct<typename T::value_type>(&value), dictionary(d)
	{
	}

	template<typename T>
	void
	ModelPartForDictionaryElementInserter<T>::Complete()
	{
		dictionary->insert(value);
	}

	// ModelPartForDictionary
	template<typename T> ModelPartForDictionary<T>::ModelPartForDictionary(T * d) : ModelPartModel<T>(d) { }

	template<typename T>
	void
	ModelPartForDictionary<T>::OnEachChild(const ChildHandler & ch)
	{
		BOOST_ASSERT(this->Model);
		for (auto & pair : *this->Model) {
			ch(pairName, std::make_shared<ModelPartForStruct<typename T::value_type>>(&pair), NULL);
		}
	}

	template<typename T>
	ChildRef
	ModelPartForDictionary<T>::GetAnonChildRef(const HookFilter &)
	{
		BOOST_ASSERT(this->Model);
		return ChildRef(std::make_shared<ModelPartForDictionaryElementInserter<T>>(this->Model));
	}

	template<typename T>
	ChildRef
	ModelPartForDictionary<T>::GetChildRef(const std::string & name, const HookFilter &, bool matchCase)
	{
		BOOST_ASSERT(this->Model);
		if (!optionalCaseEq(name, pairName, matchCase)) {
			throw IncorrectElementName(name);
		}
		return ChildRef(std::make_shared<ModelPartForDictionaryElementInserter<T>>(this->Model));
	}

	template<typename T>
	const Metadata &
	ModelPartForDictionary<T>::GetMetadata() const
	{
		return metadata;
	}

	template<typename T>
	ModelPartPtr
	ModelPartForDictionary<T>::GetContainedModelPart()
	{
		return std::make_shared<ModelPartForStruct<typename T::value_type>>(nullptr);
	}

	// ModelPartForStream
	template<typename T> ModelPartForStream<T>::ModelPartForStream(Stream<T> * s) : ModelPartModel<Stream<T>>(s) { }

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
		ModelPartForStreamRootBase(std::make_shared<ModelPartForStream<T>>(s))
	{
	}

	template<typename T>
	const std::string &
	ModelPartForStreamRoot<T>::GetRootName() const
	{
		return ModelPartForRoot<std::vector<T>>::rootName;
	}
}
#ifdef __clang__
#	pragma clang diagnostic pop
#endif

#endif
