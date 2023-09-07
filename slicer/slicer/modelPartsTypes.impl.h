#pragma once
// Keep this, there's a check its not defined in a test
#define SLICER_MODELPARTSTYPES_IMPL_H

#include "common.h"
#include "enumMap.h" // IWYU pragma: keep
#include "hookMap.h"
#include "metadata.h"
#include "modelParts.h"
#include "modelPartsTraits.h"
#include "modelPartsTypes.h"
#include <Ice/Config.h>
#include <Ice/InputStream.h>
#include <Ice/Optional.h>
#include <Ice/OutputStream.h>
#include <Ice/StreamHelpers.h>
#include <IceUtil/Config.h>
#include <boost/assert.hpp>
#include <c++11Helpers.h>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>
#include <visibility.h>

// IWYU pragma: no_forward_declare Slicer::EnumMap
namespace Ice {
	class Object;
}

#define CUSTOMMODELPARTFOR(Type, BaseModelPart, ModelPartType) \
	template<> \
	DLL_PUBLIC void ModelPart::Make<ModelPartType>( \
			typename ModelPartType::element_type * t, const ModelPartHandler & h) \
	{ \
		h(ModelPartType(t)); \
	} \
	template<> \
	DLL_PUBLIC void ModelPart::Make<ModelPartForOptional<ModelPartType>>( \
			Ice::optional<typename ModelPartType::element_type> * t, const ModelPartHandler & h) \
	{ \
		h(ModelPartForOptional<ModelPartType>(t)); \
	} \
	template<> DLL_PUBLIC void ModelPart::CreateFor(Type * s, const ModelPartHandler & h) \
	{ \
		return Make<ModelPartType>(s, h); \
	} \
	template<> DLL_PUBLIC void ModelPart::CreateFor(const Type * s, const ModelPartHandler & h) \
	{ \
		return CreateFor(const_cast<Type *>(s), h); \
	} \
	template<> DLL_PUBLIC void ModelPart::CreateFor(Ice::optional<Type> * s, const ModelPartHandler & h) \
	{ \
		return Make<ModelPartForOptional<ModelPartType>>(s, h); \
	} \
	template<> DLL_PUBLIC void ModelPart::CreateFor(const Ice::optional<Type> * s, const ModelPartHandler & h) \
	{ \
		return CreateFor(const_cast<Ice::optional<Type> *>(s), h); \
	} \
	template<> DLL_PUBLIC void ModelPart::OnRootFor(Type & s, const ModelPartRootHandler & h) \
	{ \
		return CreateFor(&s, [&s, &h](auto && mp) { \
			h(ModelPartForRoot<Type>(&s, mp)); \
		}); \
	} \
	template<> DLL_PUBLIC void ModelPart::OnRootFor(Ice::optional<Type> & s, const ModelPartRootHandler & h) \
	{ \
		return CreateFor(&s, [&s, &h](auto && mp) { \
			h(ModelPartForRoot<Ice::optional<Type>>(&s, mp)); \
		}); \
	} \
	template<> DLL_PUBLIC void ModelPart::OnRootFor(const Type & s, const ModelPartRootHandler & h) \
	{ \
		return OnRootFor(const_cast<Type &>(s), h); \
	} \
	template<> DLL_PUBLIC void ModelPart::OnRootFor(const Ice::optional<Type> & s, const ModelPartRootHandler & h) \
	{ \
		return OnRootFor(const_cast<Ice::optional<Type> &>(s), h); \
	} \
	template class BaseModelPart; \
	template class ModelPartForRoot<Type>; \
	template class ModelPartForRoot<Ice::optional<Type>>;

#define MODELPARTFOR(Type, ModelPartType) CUSTOMMODELPARTFOR(Type, ModelPartType<Type>, ModelPartType<Type>)
#define MODELPARTFORSTREAM(StreamImpl) \
	namespace Slicer { \
		template<> \
		DLL_PUBLIC void \
		ModelPart::OnRootFor(const StreamImpl & stream, const ModelPartRootHandler & h) \
		{ \
			ModelPartForStream<typename StreamImpl::element_type> strm(const_cast<StreamImpl *>(&stream)); \
			h(ModelPartForStreamRoot<typename StreamImpl::element_type>(strm)); \
		} \
	}

#ifdef ICE_CPP11_MAPPING // C++11 mapping
#	define FORWARD_ENUM(name) enum class ICE_CLASS(JAM_DLL_PUBLIC) name : unsigned char;
#else // C++98 mapping
#	define FORWARD_ENUM(name) enum ICE_CLASS(JAM_DLL_PUBLIC) name;
#endif

#ifdef __clang__
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wundefined-var-template"
#endif
namespace Slicer {
	// ModelPartForRoot
	template<typename T>
	ModelPartForRoot<T>::ModelPartForRoot(T * o, ModelPartParam omp) : ModelPartForRootBase(omp), ModelObject(o)
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

	template<typename T, typename M, Ice::optional<T> M::*MV>
	bool
	ModelPartForConverted<Ice::optional<T>, M, MV>::HasValue() const
	{
		BOOST_ASSERT(this->Model);
		return !!*this->Model;
	}

	template<typename ET, typename MT, typename Conv>
	inline bool
	ModelPartForConvertedBase::tryConvertFrom(const ValueSource & vsp, MT * model, const Conv & conv)
	{
		if (auto vspt = dynamic_cast<const TValueSource<ET> *>(&vsp)) {
			using CA = callable_param<Conv, 0>;
			ET tmp;
			vspt->set(tmp);
			if (auto converted = conv(Coerce<CA>()(tmp)); Coerce<MT>::valueExists(converted)) {
				*model = Coerce<MT>()(std::move(converted));
			}
			return true;
		}
		return false;
	}

	template<typename ET, typename MT>
	inline bool
	ModelPartForConvertedBase::tryConvertFrom(const ValueSource & vsp, MT * model)
	{
		if (auto vspt = dynamic_cast<const TValueSource<ET> *>(&vsp)) {
			if (Coerce<ET>::valueExists(*model)) {
				vspt->set(Coerce<ET>()(*model));
			}
			return true;
		}
		return false;
	}

	template<typename ET, typename MT, typename Conv>
	inline TryConvertResult
	ModelPartForConvertedBase::tryConvertTo(const ValueTarget & vsp, const MT * model, const Conv & conv)
	{
		if (auto vspt = dynamic_cast<const TValueTarget<ET> *>(&vsp)) {
			using CA = callable_param<Conv, 0>;
			if (Coerce<std::decay_t<CA>>::valueExists(*model)) {
				if (auto converted = conv(Coerce<CA>()(*model)); Coerce<ET>::valueExists(converted)) {
					vspt->get(Coerce<ET>()(std::move(converted)));
					return TryConvertResult::Value;
				}
			}
			return TryConvertResult::NoValue;
		}
		return TryConvertResult::NoAction;
	}

	template<typename ET, typename MT>
	inline TryConvertResult
	ModelPartForConvertedBase::tryConvertTo(const ValueTarget & vsp, const MT * model)
	{
		if (auto vspt = dynamic_cast<const TValueTarget<ET> *>(&vsp)) {
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
			modelPart = &modelPartOwner.emplace(&**this->Model);
		}
	}

	template<typename T>
	bool
	ModelPartForOptional<T>::hasModel() const
	{
		BOOST_ASSERT(this->Model);
		return !!*this->Model;
	}

	template<typename T>
	void
	ModelPartForOptional<T>::Create()
	{
		BOOST_ASSERT(this->Model);
		if (!*this->Model) {
			*this->Model = typename T::element_type();
			modelPart = &modelPartOwner.emplace(&**this->Model);
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
	void
	ModelPartForComplex<T>::OnEachChild(const ChildHandler & ch)
	{
		for (const auto & h : hooks()) {
			h->On(GetModel(), [h, &ch](auto && mp) {
				h->apply(ch, mp);
			});
		}
	}

	template<typename T>
	template<typename R>
	bool
	ModelPartForComplex<T>::OnChildFromRange(const SubPartHandler & ch, const R & range, const HookFilter & flt)
	{
		const auto itr = std::find_if(range.begin(), range.end(), [&flt](auto && h) {
			return h->filter(flt);
		});
		if (itr != range.end()) {
			const auto & h = *itr;
			h->On(GetModel(), [h, &ch](auto && mp) {
				ch(mp, h->GetMetadata());
			});
			return true;
		}
		return false;
	}

	template<typename T>
	bool
	ModelPartForComplex<T>::OnAnonChild(const SubPartHandler & ch, const HookFilter & flt)
	{
		return OnChildFromRange(ch, hooks(), flt);
	}

	template<typename T>
	bool
	ModelPartForComplex<T>::OnChild(
			const SubPartHandler & ch, std::string_view name, const HookFilter & flt, MatchCase matchCase)
	{
		switch (matchCase) {
			case MatchCase::Yes:
				return OnChildFromRange(ch, hooks().equal_range(name), flt);
			case MatchCase::No: {
				std::string i {name};
				to_lower(i);
				return OnChildFromRange(ch, hooks().equal_range_lower(i), flt);
			}
			case MatchCase::No_Prelowered:
				return OnChildFromRange(ch, hooks().equal_range_lower(name), flt);
		}
#ifdef __cpp_lib_unreachable
		std::unreachable();
#else
		__builtin_unreachable();
#endif
	}

	template<typename T> class DLL_PRIVATE ModelPartForComplex<T>::HookBase : public HookCommon {
	public:
		using HookCommon::HookCommon;

		virtual void On(T * t, const ModelPartHandler &) const = 0;
	};

	template<typename T>
	template<typename MT, typename MP, std::size_t N>
	class DLL_PRIVATE ModelPartForComplex<T>::Hook : public ModelPartForComplex<T>::HookBase {
	public:
		template<typename... MD>
		constexpr Hook(MT T::*m, std::string_view n, std::string_view nl, const std::string * ns, MD &&... md) :
			HookBase(n, nl, ns), member(m), hookMetadata {{std::forward<MD>(md)...}}
		{
			static_assert(sizeof...(MD) == N, "Wrong amount of metadata");
		}

		~Hook() override = default;
		SPECIAL_MEMBERS_DEFAULT(Hook);

		[[nodiscard]] const Metadata &
		GetMetadata() const override
		{
			return hookMetadata;
		}

		void
		On(T * t, const ModelPartHandler & h) const override
		{
			if (t) {
				return Make<MP>(&(t->*member), h);
			}
			return Make<MP>(nullptr, h);
		}

	private:
		using MemPtr = MT T::*;
		const MemPtr member;
		const MetaDataImpl<N> hookMetadata;
	};

	// ModelPartForClass
	template<typename T>
	void
	ModelPartForClass<T>::Create()
	{
		BOOST_ASSERT(this->Model);
		if constexpr (std::is_abstract_v<T>) {
			throw AbstractClassException(ModelPartForComplexBase::demangle(typeid(T).name()));
		}
		else {
			*this->Model = std::make_shared<T>();
		}
	}

	template<typename T>
	T *
	ModelPartForClass<T>::GetModel()
	{
		return this->Model ? this->Model->get() : nullptr;
	}

	template<typename T>
	void
	ModelPartForClass<T>::OnSubclass(const ModelPartHandler & h, const std::string & name)
	{
		BOOST_ASSERT(this->Model);
		return ModelPartForComplexBase::onSubclass(name, this->Model, h);
	}

	template<typename T>
	bool
	ModelPartForClass<T>::HasValue() const
	{
		BOOST_ASSERT(this->Model);
		return !!*this->Model;
	}

	template<typename T>
	std::optional<std::string>
	ModelPartForClass<T>::GetTypeIdProperty() const
	{
		return typeIdProperty;
	}

	template<typename T>
	void
	ModelPartForClass<T>::CreateModelPart(void * p, const ModelPartHandler & h)
	{
		return ::Slicer::ModelPart::CreateFor(static_cast<element_type *>(p), h);
	}

	template<typename T>
	void
	ModelPartForClass<T>::registerClass()
	{
		ModelPartForComplexBase::registerClass(className, typeName, &ModelPartForClass<T>::CreateModelPart);
	}

	template<typename T>
	void
	ModelPartForClass<T>::unregisterClass()
	{
		ModelPartForComplexBase::unregisterClass(className, typeName);
	}

	template<typename T>
	TypeId
	ModelPartForClass<T>::GetTypeId() const
	{
		BOOST_ASSERT(this->Model);
		return ModelPartForComplexBase::getTypeId(
				[this]() {
					if constexpr (std::is_base_of_v<Ice::Object, T>) {
						return (*this->Model)->ice_id();
					}
					else {
						return ModelPartForComplexBase::demangle(typeid(*this->Model->get()).name());
					}
				}(),
				className);
	}

	// ModelPartForStruct
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
	template<typename T>
	const Metadata &
	ModelPartForEnum<T>::GetMetadata() const
	{
		return metadata;
	}

	template<typename Ex, typename ExP, typename T, typename V>
	inline const auto *
	ModelPartForEnumLookup(const EnumMap<T> & enumerations, const V & val)
	{
		if (auto i = enumerations.find(val)) {
			return i;
		}
		throw Ex(ExP(val), typeid(T).name());
	}

	template<typename T>
	T
	ModelPartForEnum<T>::lookup(std::string_view val)
	{
		return ModelPartForEnumLookup<InvalidEnumerationSymbol, std::string, T>(enumerations(), val)->value;
	}

	template<typename T>
	const std::string &
	ModelPartForEnum<T>::lookup(T val)
	{
		return *ModelPartForEnumLookup<InvalidEnumerationValue, ::Ice::Int, T>(enumerations(), val)->nameStr;
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
	template<typename T>
	void
	ModelPartForSequence<T>::OnEachChild(const ChildHandler & ch)
	{
		BOOST_ASSERT(this->Model);
		for (auto & element : *this->Model) {
			ModelPart::CreateFor(&element, [&ch](auto && mp) {
				ch(elementName, mp, nullptr);
			});
		}
	}

	template<typename T>
	bool
	ModelPartForSequence<T>::OnAnonChild(const SubPartHandler & ch, const HookFilter &)
	{
		BOOST_ASSERT(this->Model);
		ModelPart::CreateFor(&this->Model->emplace_back(), [&ch](auto && mp) {
			ch(mp, emptyMetadata);
		});
		return true;
	}

	template<typename T>
	const Metadata &
	ModelPartForSequence<T>::GetMetadata() const
	{
		return metadata;
	}

	template<typename T>
	const std::string &
	ModelPartForSequence<T>::GetElementName() const
	{
		return elementName;
	}

	template<typename T>
	void
	ModelPartForSequence<T>::OnContained(const ModelPartHandler & h)
	{
		return ModelPart::CreateFor<typename T::value_type>(nullptr, h);
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
	template<typename T>
	void
	ModelPartForDictionary<T>::OnEachChild(const ChildHandler & ch)
	{
		BOOST_ASSERT(this->Model);
		for (auto & pair : *this->Model) {
			ch(pairName, ModelPartForStruct<typename T::value_type>(&pair), nullptr);
		}
	}

	template<typename T>
	bool
	ModelPartForDictionary<T>::OnAnonChild(const SubPartHandler & ch, const HookFilter &)
	{
		BOOST_ASSERT(this->Model);
		ch(ModelPartForDictionaryElementInserter<T>(this->Model), emptyMetadata);
		return true;
	}

	template<typename T>
	bool
	ModelPartForDictionary<T>::OnChild(
			const SubPartHandler & ch, std::string_view name, const HookFilter &, MatchCase matchCase)
	{
		BOOST_ASSERT(this->Model);
		if (!optionalCaseEq(name, pairName, matchCase == MatchCase::Yes)) {
			throw IncorrectElementName(std::string {name});
		}
		ch(ModelPartForDictionaryElementInserter<T>(this->Model), emptyMetadata);
		return true;
	}

	template<typename T>
	const Metadata &
	ModelPartForDictionary<T>::GetMetadata() const
	{
		return metadata;
	}

	template<typename T>
	void
	ModelPartForDictionary<T>::OnContained(const ModelPartHandler & h)
	{
		return h(ModelPartForStruct<typename T::value_type>(nullptr));
	}

	// ModelPartForStream
	template<typename T>
	void
	ModelPartForStream<T>::OnContained(const ModelPartHandler & h)
	{
		ModelPart::CreateFor<T>(nullptr, h);
	}

	template<typename T>
	void
	ModelPartForStream<T>::OnEachChild(const ChildHandler & ch)
	{
		BOOST_ASSERT(this->Model);
		this->Model->Produce([&ch](const T & element) {
			ModelPart::CreateFor(&element, [&ch](auto && mp) {
				ch(ModelPartForSequence<std::vector<T>>::elementName, mp, nullptr);
			});
		});
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
