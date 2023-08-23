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
#include <cxxabi.h>
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
	using ClassRefMap = std::map<std::string, ClassRef, std::less<>>;
	using ClassNamePair = std::pair<std::string, std::string>;
	using ClassNameMap = boost::multi_index_container<ClassNamePair,
			boost::multi_index::indexed_by<
					boost::multi_index::ordered_unique<
							boost::multi_index::member<ClassNamePair, const std::string, &ClassNamePair::first>,
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

	const std::string &
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

	template<> const std::string Slicer::ModelPartForRoot<std::string>::rootName = "String";
	template<> const std::string Slicer::ModelPartForRoot<bool>::rootName = "Boolean";
	template<> const std::string Slicer::ModelPartForRoot<Ice::Float>::rootName = "Float";
	template<> const std::string Slicer::ModelPartForRoot<Ice::Double>::rootName = "Double";
	template<> const std::string Slicer::ModelPartForRoot<Ice::Byte>::rootName = "Byte";
	template<> const std::string Slicer::ModelPartForRoot<Ice::Short>::rootName = "Short";
	template<> const std::string Slicer::ModelPartForRoot<Ice::Int>::rootName = "Int";
	template<> const std::string Slicer::ModelPartForRoot<Ice::Long>::rootName = "Long";

	template<> const std::string Slicer::ModelPartForRoot<Ice::optional<std::string>>::rootName = "OptionalString";
	template<> const std::string Slicer::ModelPartForRoot<Ice::optional<bool>>::rootName = "OptionalBoolean";
	template<> const std::string Slicer::ModelPartForRoot<Ice::optional<Ice::Float>>::rootName = "OptionalFloat";
	template<> const std::string Slicer::ModelPartForRoot<Ice::optional<Ice::Double>>::rootName = "OptionalDouble";
	template<> const std::string Slicer::ModelPartForRoot<Ice::optional<Ice::Byte>>::rootName = "OptionalByte";
	template<> const std::string Slicer::ModelPartForRoot<Ice::optional<Ice::Short>>::rootName = "OptionalShort";
	template<> const std::string Slicer::ModelPartForRoot<Ice::optional<Ice::Int>>::rootName = "OptionalInt";
	template<> const std::string Slicer::ModelPartForRoot<Ice::optional<Ice::Long>>::rootName = "OptionalLong";

	MODELPARTFOR(std::string, ModelPartForSimple)
	MODELPARTFOR(bool, ModelPartForSimple)
	MODELPARTFOR(Ice::Float, ModelPartForSimple)
	MODELPARTFOR(Ice::Double, ModelPartForSimple)
	MODELPARTFOR(Ice::Byte, ModelPartForSimple)
	MODELPARTFOR(Ice::Short, ModelPartForSimple)
	MODELPARTFOR(Ice::Int, ModelPartForSimple)
	MODELPARTFOR(Ice::Long, ModelPartForSimple)

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
			const SubPartHandler & h, std::string_view name, const HookFilter & hf, bool matchCase)
	{
		if (!optionalCaseEq(name, GetRootName(), matchCase)) {
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
			const std::string & className, const std::string * typeName, const ClassRef & cr)
	{
		refs->emplace(className, cr);
		if (typeName) {
			names->emplace(className, *typeName);
		}
	}

	void
	ModelPartForComplexBase::unregisterClass(const std::string & className, const std::string * typeName)
	{
		refs->erase(className);
		if (typeName) {
			names->get<0>().erase(className);
		}
	}

	void
	ModelPartForComplexBase::onSubclass(const std::string & name, void * m, const ModelPartHandler & h)
	{
		if (const auto ref = refs->find(ToModelTypeName(name)); ref != refs->end()) {
			return ref->second(m, h);
		}
		throw UnknownType(name);
	}

	TypeId
	ModelPartForComplexBase::getTypeId(const std::string & id, const std::string & className)
	{
		return (id == className) ? TypeId() : ToExchangeTypeName(id);
	}

	std::string
	ModelPartForComplexBase::demangle(const char * mangled)
	{
		auto buf = std::unique_ptr<char, decltype(free) *>(
				abi::__cxa_demangle(mangled, nullptr, nullptr, nullptr), std::free);
		return "::" + std::string(buf.get());
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
			const SubPartHandler & h, std::string_view name, const HookFilter & flt, bool matchCase)
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
			const SubPartHandler & h, std::string_view name, const HookFilter & flt, bool matchCase)
	{
		if (!name.empty() && !optionalCaseEq(name, GetElementName(), matchCase)) {
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
