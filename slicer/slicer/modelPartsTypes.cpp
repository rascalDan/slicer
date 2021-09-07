#include "modelPartsTypes.impl.h"
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>
#include <cxxabi.h>

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

	static void createClassMaps() __attribute__((constructor(208)));
	static void deleteClassMaps() __attribute__((destructor(208)));
	static ClassNameMap * names;
	static ClassRefMap * refs;

	void
	createClassMaps()
	{
		names = new ClassNameMap();
		refs = new ClassRefMap();
	}

	static void
	deleteClassMaps()
	{
		delete names;
		delete refs;
		names = nullptr;
		refs = nullptr;
	}

	ClassNameMap *
	classNameMap()
	{
		return names;
	}

	ClassRefMap *
	classRefMap()
	{
		return refs;
	}

	const std::string &
	ModelPartForComplexBase::ToModelTypeName(const std::string & name)
	{
		auto & right = classNameMap()->get<1>();
		auto mapped = right.find(name);
		if (mapped != right.end()) {
			return mapped->first;
		}
		return name;
	}

	const std::string &
	ModelPartForComplexBase::ToExchangeTypeName(const std::string & name)
	{
		auto & left = classNameMap()->get<0>();
		auto mapped = left.find(name);
		if (mapped != left.end()) {
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

	MODELPARTFOR(std::string, ModelPartForSimple);
	MODELPARTFOR(bool, ModelPartForSimple);
	MODELPARTFOR(Ice::Float, ModelPartForSimple);
	MODELPARTFOR(Ice::Double, ModelPartForSimple);
	MODELPARTFOR(Ice::Byte, ModelPartForSimple);
	MODELPARTFOR(Ice::Short, ModelPartForSimple);
	MODELPARTFOR(Ice::Int, ModelPartForSimple);
	MODELPARTFOR(Ice::Long, ModelPartForSimple);

	bool
	optionalCaseEq(std::string_view a, std::string_view b, bool matchCase)
	{
		return (matchCase ? boost::equals(a, b) : boost::iequals(a, b));
	}

	// ModelPartForRootBase
	ModelPartForRootBase::ModelPartForRootBase(ModelPartPtr m) : mp(std::move(m)) { }

	ChildRef
	ModelPartForRootBase::GetAnonChildRef(const HookFilter &)
	{
		mp->Create();
		return ChildRef(mp);
	}

	ChildRef
	ModelPartForRootBase::GetChildRef(std::string_view name, const HookFilter & hf, bool matchCase)
	{
		if (!optionalCaseEq(name, GetRootName(), matchCase)) {
			throw IncorrectElementName(std::string {name});
		}
		return GetAnonChildRef(hf);
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

	ModelPartPtr
	ModelPartForRootBase::GetContainedModelPart()
	{
		return mp->GetContainedModelPart();
	}

	void
	ModelPartForSimpleBase::OnEachChild(const ChildHandler &)
	{
	}
	ChildRef
	ModelPartForSimpleBase::GetAnonChildRef(const HookFilter &)
	{
		return ChildRef();
	}
	ChildRef
	ModelPartForSimpleBase::GetChildRef(std::string_view, const HookFilter &, bool)
	{
		return ChildRef();
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

	void
	ModelPartForConvertedBase::OnEachChild(const ChildHandler &)
	{
	}
	ChildRef
	ModelPartForConvertedBase::GetAnonChildRef(const HookFilter &)
	{
		return ChildRef();
	}
	ChildRef
	ModelPartForConvertedBase::GetChildRef(std::string_view, const HookFilter &, bool)
	{
		return ChildRef();
	}
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
		Slicer::classRefMap()->insert({className, cr});
		if (typeName) {
			Slicer::classNameMap()->insert({className, *typeName});
		}
	}
	void
	ModelPartForComplexBase::unregisterClass(const std::string & className, const std::string * typeName)
	{
		Slicer::classRefMap()->erase(className);
		if (typeName) {
			classNameMap()->get<0>().erase(className);
		}
	}
	ModelPartPtr
	ModelPartForComplexBase::getSubclassModelPart(const std::string & name, void * m)
	{
		auto ref = classRefMap()->find(ToModelTypeName(name));
		if (ref == classRefMap()->end()) {
			throw UnknownType(name);
		}
		return ref->second(m);
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

	ChildRef
	ModelPartForOptionalBase::GetAnonChildRef(const HookFilter & flt)
	{
		if (this->hasModel()) {
			return modelPart->GetAnonChildRef(flt);
		}
		return ChildRef();
	}

	ChildRef
	ModelPartForOptionalBase::GetChildRef(std::string_view name, const HookFilter & flt, bool matchCase)
	{
		if (this->hasModel()) {
			return modelPart->GetChildRef(name, flt, matchCase);
		}
		return ChildRef();
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
	};

	const Metadata &
	ModelPartForOptionalBase::GetMetadata() const
	{
		return modelPart->GetMetadata();
	}

	void
	ModelPartForEnumBase::OnEachChild(const ChildHandler &)
	{
	}
	ChildRef
	ModelPartForEnumBase::GetAnonChildRef(const HookFilter &)
	{
		return ChildRef();
	}
	ChildRef
	ModelPartForEnumBase::GetChildRef(std::string_view, const HookFilter &, bool)
	{
		return ChildRef();
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
	ChildRef
	ModelPartForSequenceBase::GetChildRef(std::string_view name, const HookFilter & flt, bool matchCase)
	{
		if (!name.empty() && !optionalCaseEq(name, GetElementName(), matchCase)) {
			throw IncorrectElementName(std::string {name});
		}
		return GetAnonChildRef(flt);
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
	// NOLINTNEXTLINE(hicpp-no-array-decay)
	ChildRef
	ModelPartForStreamBase::GetAnonChildRef(const Slicer::HookFilter &)
	{
		throw InvalidStreamOperation(__FUNCTION__);
	}
	// NOLINTNEXTLINE(hicpp-no-array-decay)
	ChildRef
	ModelPartForStreamBase::GetChildRef(std::string_view, const Slicer::HookFilter &, bool)
	{
		throw InvalidStreamOperation(__FUNCTION__);
	}
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
	// Stream Roots
	ModelPartForStreamRootBase::ModelPartForStreamRootBase(const ModelPartPtr & mp) : ModelPartForRootBase(mp) { }
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
