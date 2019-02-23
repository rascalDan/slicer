#include "modelPartsTypes.impl.h"
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <cxxabi.h>

namespace Slicer {
	typedef std::map<std::string, ClassRef, std::less<>> ClassRefMap;
	typedef boost::multi_index_container<
		std::pair<std::string, std::string>,
		boost::multi_index::indexed_by<
			boost::multi_index::ordered_unique<boost::multi_index::member<std::pair<std::string, std::string>, const std::string, &std::pair<std::string, std::string>::first>, std::less<>>,
			boost::multi_index::ordered_unique<boost::multi_index::member<std::pair<std::string, std::string>, const std::string, &std::pair<std::string, std::string>::second>, std::less<>>
		>> ClassNameMap;

	static void createClassMaps() __attribute__((constructor(208)));
	static void deleteClassMaps() __attribute__((destructor(208)));
	static ClassNameMap * names;
	static ClassRefMap * refs;

	void createClassMaps()
	{
		names = new ClassNameMap();
		refs = new ClassRefMap();
	}

	static void deleteClassMaps()
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
	optionalCaseEq(const std::string & a, const std::string & b, bool matchCase)
	{
		return (matchCase ? boost::equals(a, b) : boost::iequals(a, b));
	}

	// ModelPartForRootBase
	ModelPartForRootBase::ModelPartForRootBase(ModelPartPtr m) :
		mp(m)
	{
	}

	ChildRef
	ModelPartForRootBase::GetAnonChildRef(const HookFilter &)
	{
		mp->Create();
		return ChildRef(mp);
	}

	ChildRef
	ModelPartForRootBase::GetChildRef(const std::string & name, const HookFilter & hf, bool matchCase)
	{
		if (!optionalCaseEq(name, GetRootName(), matchCase)) {
			throw IncorrectElementName(name);
		}
		return GetAnonChildRef(hf);
	}

	void ModelPartForRootBase::OnEachChild(const ChildHandler & ch)
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

	void ModelPartForSimpleBase::OnEachChild(const ChildHandler &) { }
	ChildRef ModelPartForSimpleBase::GetAnonChildRef(const HookFilter &) { return ChildRef(); }
	ChildRef ModelPartForSimpleBase::GetChildRef(const std::string &, const HookFilter &, bool) { return ChildRef(); }
	bool ModelPartForSimpleBase::HasValue() const { return true; }
	ModelPartType ModelPartForSimpleBase::GetType() const { return type; }
	const ModelPartType ModelPartForSimpleBase::type = mpt_Simple;

	void ModelPartForConvertedBase::OnEachChild(const ChildHandler &) { }
	ChildRef ModelPartForConvertedBase::GetAnonChildRef(const HookFilter &) { return ChildRef(); }
	ChildRef ModelPartForConvertedBase::GetChildRef(const std::string &, const HookFilter &, bool) { return ChildRef(); }
	bool ModelPartForConvertedBase::HasValue() const { return true; }
	ModelPartType ModelPartForConvertedBase::GetType() const { return type; }
	const ModelPartType ModelPartForConvertedBase::type = mpt_Simple;

	ModelPartType ModelPartForComplexBase::GetType() const { return type; }
	const ModelPartType ModelPartForComplexBase::type = mpt_Complex;
	void ModelPartForComplexBase::registerClass(const std::string & className, const std::string * typeName, const ClassRef & cr)
	{
		Slicer::classRefMap()->insert({ className, cr });
		if (typeName) {
			Slicer::classNameMap()->insert({ className, *typeName });
		}
	}
	void ModelPartForComplexBase::unregisterClass(const std::string & className, const std::string * typeName)
	{
		Slicer::classRefMap()->erase(className);
		if (typeName) {
			classNameMap()->get<0>().erase(className);
		}
	}
	ModelPartPtr ModelPartForComplexBase::getSubclassModelPart(const std::string & name, void * m)
	{
		auto ref = classRefMap()->find(ToModelTypeName(name));
		if (ref == classRefMap()->end()) {
			throw UnknownType(name);
		}
		return ref->second(m);
	}
	TypeId ModelPartForComplexBase::GetTypeId(const std::string & id, const std::string & className)
	{
		return (id == className) ? TypeId() : ToExchangeTypeName(id);
	}

	std::string ModelPartForComplexBase::demangle(const char * mangled)
	{
		auto buf = std::unique_ptr<char, decltype(free)*>(abi::__cxa_demangle(mangled, NULL, NULL, NULL), std::free);
		return "::" + std::string(buf.get());
	}

	void ModelPartForOptionalBase::OnEachChild(const ChildHandler & ch)
	{
		if (this->hasModel()) {
			modelPart->OnEachChild(ch);
		}
	}

	void ModelPartForOptionalBase::Complete()
	{
		if (this->hasModel()) {
			modelPart->Complete();
		}
	}

	ChildRef ModelPartForOptionalBase::GetAnonChildRef(const HookFilter & flt)
	{
		if (this->hasModel()) {
			return modelPart->GetAnonChildRef(flt);
		}
		return ChildRef();
	}

	ChildRef ModelPartForOptionalBase::GetChildRef(const std::string & name, const HookFilter & flt, bool matchCase)
	{
		if (this->hasModel()) {
			return modelPart->GetChildRef(name, flt, matchCase);
		}
		return ChildRef();
	}

	void ModelPartForOptionalBase::SetValue(ValueSource && s)
	{
		if (this->hasModel()) {
			modelPart->SetValue(std::move(s));
		}
	}

	bool ModelPartForOptionalBase::HasValue() const
	{
		return this->hasModel() && modelPart->HasValue();
	}

	bool ModelPartForOptionalBase::IsOptional() const
	{
		return true;
	};

	const Metadata & ModelPartForOptionalBase::GetMetadata() const
	{
		return modelPart->GetMetadata();
	}

	void ModelPartForEnumBase::OnEachChild(const ChildHandler &) { }
	ChildRef ModelPartForEnumBase::GetAnonChildRef(const HookFilter &) { return ChildRef(); }
	ChildRef ModelPartForEnumBase::GetChildRef(const std::string &, const HookFilter &, bool) { return ChildRef(); }
	bool ModelPartForEnumBase::HasValue() const { return true; }
	ModelPartType ModelPartForEnumBase::GetType() const { return type; }
	const ModelPartType ModelPartForEnumBase::type = mpt_Simple;

	bool ModelPartForSequenceBase::HasValue() const { return true; }
	ModelPartType ModelPartForSequenceBase::GetType() const { return type; }
	const ModelPartType ModelPartForSequenceBase::type = mpt_Sequence;

	bool ModelPartForDictionaryBase::HasValue() const { return true; }
	ModelPartType ModelPartForDictionaryBase::GetType() const { return type; }
	const ModelPartType ModelPartForDictionaryBase::type = mpt_Dictionary;

	// Streams
	// NOLINTNEXTLINE(hicpp-no-array-decay)
	ChildRef ModelPartForStreamBase::GetAnonChildRef(const Slicer::HookFilter &) { throw InvalidStreamOperation(__FUNCTION__); }
	// NOLINTNEXTLINE(hicpp-no-array-decay)
	ChildRef ModelPartForStreamBase::GetChildRef(const std::string &, const Slicer::HookFilter &, bool) { throw InvalidStreamOperation(__FUNCTION__); }
	ModelPartType ModelPartForStreamBase::GetType() const { return mpt_Sequence; }
	bool ModelPartForStreamBase::HasValue() const { return true; }
	// Stream Roots
	ModelPartForStreamRootBase::ModelPartForStreamRootBase(ModelPartPtr mp) : ModelPartForRootBase(mp) { }
	// NOLINTNEXTLINE(hicpp-no-array-decay)
	void ModelPartForStreamRootBase::Write(Ice::OutputStream&) const { throw InvalidStreamOperation(__FUNCTION__); }
	// NOLINTNEXTLINE(hicpp-no-array-decay)
	void ModelPartForStreamRootBase::Read(Ice::InputStream&) { throw InvalidStreamOperation(__FUNCTION__); }
	bool ModelPartForStreamRootBase::HasValue() const { return mp->HasValue(); }
	void ModelPartForStreamRootBase::OnEachChild(const ChildHandler & ch) { ch(GetRootName(), mp, NULL); }
}

