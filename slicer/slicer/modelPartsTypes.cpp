#include "modelPartsTypes.h"
#include "common.h"
#include "modelParts.h"
#include "modelPartsTypes.impl.h"
#include <Ice/Config.h>
#include <Ice/Optional.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>
#include <cstdlib>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

// IWYU pragma: no_forward_declare boost::multi_index::member
// IWYU pragma: no_include <boost/operators.hpp>

namespace Ice {
	class InputStream;
	class OutputStream;
}

namespace Slicer {
	static_assert(!isOptional<int>::value);
	static_assert(isOptional<::Ice::optional<int>>::value);
	static_assert(isOptional<::IceUtil::Optional<int>>::value);

	using ClassRefMap = std::map<std::string, const ClassRefBase *, std::less<>>;
	using ClassNamePair = std::pair<std::string_view, std::string>;
	using ClassNameMap = boost::multi_index_container<ClassNamePair,
			boost::multi_index::indexed_by<
					boost::multi_index::ordered_unique<
							boost::multi_index::member<ClassNamePair, const std::string_view, &ClassNamePair::first>,
							std::less<>>,
					boost::multi_index::ordered_unique<
							boost::multi_index::member<ClassNamePair, const std::string, &ClassNamePair::second>,
							std::less<>>>>;

	namespace {
		constinit std::unique_ptr<ClassNameMap> names;
		constinit std::unique_ptr<ClassRefMap> refs;

		void createClassMaps() __attribute__((constructor(208)));

		void
		createClassMaps()
		{
			names = std::make_unique<ClassNameMap>();
			refs = std::make_unique<ClassRefMap>();
		}
	}

	std::string_view
	ModelPartForComplexBase::ToModelTypeName(const std::string & name)
	{
		const auto & right = names->get<1>();

		if (const auto mapped = right.find(name); mapped != right.end()) {
			return mapped->first;
		}
		return name;
	}

	const std::string &
	ModelPartForComplexBase::ToExchangeTypeName(const std::string & name)
	{
		const auto & left = names->get<0>();

		if (const auto mapped = left.find(name); mapped != left.end()) {
			return mapped->second;
		}
		return name;
	}

	[[noreturn]] void
	ModelPartForComplexBase::throwIncorrectType(const std::string & name, const std::type_info & target)
	{
		throw IncorrectType(name, demangle(target.name()));
	}

	[[noreturn]] void
	ModelPartForComplexBase::throwAbstractClassException(const std::type_info & target)
	{
		throw AbstractClassException(demangle(target.name()));
	}

#define Roots(Type, Name, NameLen) \
	template<> CONSTSTR(NameLen) Slicer::ModelPartForRoot<Type>::rootName {#Name}; \
	template<> \
	CONSTSTR(BOOST_PP_ADD(8, NameLen)) \
	Slicer::ModelPartForRoot<Ice::optional<Type>>::rootName {"Optional" #Name}; \
	MODELPARTFOR(Type, ModelPartForSimple)

	Roots(std::string, String, 6);
	Roots(bool, Boolean, 8);
	Roots(Ice::Float, Float, 5);
	Roots(Ice::Double, Double, 6);
	Roots(Ice::Byte, Byte, 4);
	Roots(Ice::Short, Short, 5);
	Roots(Ice::Int, Int, 3);
	Roots(Ice::Long, Long, 4);
#undef RootNames

	bool
	optionalCaseEq(std::string_view a, std::string_view b, bool matchCase)
	{
		return (matchCase ? boost::equals(a, b) : boost::iequals(a, b));
	}

	// ModelPartForRootBase
	ModelPartForRootBase::ModelPartForRootBase(ModelPartParam m) : mp(m) { }

	bool
	ModelPartForRootBase::OnAnonChild(const SubPartHandler & h, const HookFilter &)
	{
		mp->Create();
		h(mp, emptyMetadata);
		return true;
	}

	bool
	ModelPartForRootBase::OnChild(
			const SubPartHandler & h, std::string_view name, const HookFilter & hf, MatchCase matchCase)
	{
		if (!optionalCaseEq(name, GetRootName(), matchCase == MatchCase::Yes)) {
			throw IncorrectElementName(std::string {name});
		}
		return OnAnonChild(h, hf);
	}

	void
	ModelPartForRootBase::OnEachChild(const ChildHandler & ch)
	{
		ch(GetRootName(), mp, nullptr);
	}

	ModelPartType
	ModelPartForRootBase::GetType() const
	{
		return mp->GetType();
	}

	bool
	ModelPartForRootBase::IsOptional() const
	{
		return mp->IsOptional();
	}

	void
	ModelPartForRootBase::OnContained(const ModelPartHandler & h)
	{
		return mp->OnContained(h);
	}

	bool
	ModelPartForSimpleBase::HasValue() const
	{
		return true;
	}

	ModelPartType
	ModelPartForSimpleBase::GetType() const
	{
		return type;
	}

	const ModelPartType ModelPartForSimpleBase::type = ModelPartType::Simple;

	bool
	ModelPartForConvertedBase::HasValue() const
	{
		return true;
	}

	ModelPartType
	ModelPartForConvertedBase::GetType() const
	{
		return type;
	}

	void
	ModelPartForConvertedBase::conversion_fail(std::string_view typeName)
	{
		throw NoConversionFound(std::string {typeName});
	}

	const ModelPartType ModelPartForConvertedBase::type = ModelPartType::Simple;

	ModelPartType
	ModelPartForComplexBase::GetType() const
	{
		return type;
	}

	const ModelPartType ModelPartForComplexBase::type = ModelPartType::Complex;

	void
	ModelPartForComplexBase::registerClass(
			const std::string_view className, const std::optional<std::string_view> typeName, const ClassRefBase * cr)
	{
		refs->emplace(className, cr);
		if (typeName) {
			names->emplace(className, *typeName);
		}
	}

	void
	ModelPartForComplexBase::unregisterClass(
			const std::string_view className, const std::optional<std::string_view> typeName)
	{
		if (const auto i = refs->find(className); i != refs->end()) {
			refs->erase(i);
		}
		if (typeName) {
			names->get<0>().erase(className);
		}
	}

	const ClassRefBase *
	ModelPartForComplexBase::getSubclassRef(const std::string & name)
	{
		if (const auto ref = refs->find(ToModelTypeName(name)); ref != refs->end()) {
			return ref->second;
		}
		throw UnknownType(name);
	}

	TypeId
	ModelPartForComplexBase::getTypeId(const std::string & id, const std::string_view className)
	{
		return (id == className) ? TypeId() : ToExchangeTypeName(id);
	}

	void
	ModelPartForOptionalBase::OnEachChild(const ChildHandler & ch)
	{
		if (this->hasModel()) {
			modelPart->OnEachChild(ch);
		}
	}

	void
	ModelPartForOptionalBase::Complete()
	{
		if (this->hasModel()) {
			modelPart->Complete();
		}
	}

	bool
	ModelPartForOptionalBase::OnAnonChild(const SubPartHandler & h, const HookFilter & flt)
	{
		if (this->hasModel()) {
			return modelPart->OnAnonChild(h, flt);
		}
		return false;
	}

	bool
	ModelPartForOptionalBase::OnChild(
			const SubPartHandler & h, std::string_view name, const HookFilter & flt, MatchCase matchCase)
	{
		if (this->hasModel()) {
			return modelPart->OnChild(h, name, flt, matchCase);
		}
		return false;
	}

	void
	ModelPartForOptionalBase::SetValue(ValueSource && s)
	{
		if (this->hasModel()) {
			modelPart->SetValue(std::move(s));
		}
	}

	bool
	ModelPartForOptionalBase::GetValue(ValueTarget && s)
	{
		if (this->hasModel()) {
			return modelPart->GetValue(std::move(s));
		}
		return false;
	}

	bool
	ModelPartForOptionalBase::HasValue() const
	{
		return this->hasModel() && modelPart->HasValue();
	}

	bool
	ModelPartForOptionalBase::IsOptional() const
	{
		return true;
	}

	const Metadata &
	ModelPartForOptionalBase::GetMetadata() const
	{
		return modelPart->GetMetadata();
	}

	bool
	ModelPartForEnumBase::HasValue() const
	{
		return true;
	}

	ModelPartType
	ModelPartForEnumBase::GetType() const
	{
		return type;
	}

	const ModelPartType ModelPartForEnumBase::type = ModelPartType::Simple;

	bool
	ModelPartForSequenceBase::HasValue() const
	{
		return true;
	}

	ModelPartType
	ModelPartForSequenceBase::GetType() const
	{
		return type;
	}

	bool
	ModelPartForSequenceBase::OnChild(
			const SubPartHandler & h, std::string_view name, const HookFilter & flt, MatchCase matchCase)
	{
		if (!name.empty() && !optionalCaseEq(name, GetElementName(), matchCase == MatchCase::Yes)) {
			throw IncorrectElementName(std::string {name});
		}
		return OnAnonChild(h, flt);
	}

	const ModelPartType ModelPartForSequenceBase::type = ModelPartType::Sequence;

	bool
	ModelPartForDictionaryBase::HasValue() const
	{
		return true;
	}

	ModelPartType
	ModelPartForDictionaryBase::GetType() const
	{
		return type;
	}

	const ModelPartType ModelPartForDictionaryBase::type = ModelPartType::Dictionary;

	void
	ModelPartForDictionaryBase::throwIncorrectElementName(const std::string_view name)
	{
		throw IncorrectElementName(std::string {name});
	}

	// Streams
	ModelPartType
	ModelPartForStreamBase::GetType() const
	{
		return ModelPartType::Sequence;
	}

	bool
	ModelPartForStreamBase::HasValue() const
	{
		return true;
	}

	// NOLINTNEXTLINE(hicpp-no-array-decay)
	void
	ModelPartForStreamRootBase::Write(Ice::OutputStream &) const
	{
		throw InvalidStreamOperation(__FUNCTION__);
	}

	// NOLINTNEXTLINE(hicpp-no-array-decay)
	void
	ModelPartForStreamRootBase::Read(Ice::InputStream &)
	{
		throw InvalidStreamOperation(__FUNCTION__);
	}

	bool
	ModelPartForStreamRootBase::HasValue() const
	{
		return mp->HasValue();
	}

	void
	ModelPartForStreamRootBase::OnEachChild(const ChildHandler & ch)
	{
		ch(GetRootName(), mp, NULL);
	}
}
