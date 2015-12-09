#include "parser.h"
#include "metadata.h"
#include <Slice/Parser.h>
#include <Slice/Preprocessor.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <Slice/CPlusPlusUtil.h>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/convenience.hpp>
#include <mutex>
#include <fprintbf.h>
#include <safeMapFind.h>

namespace fs = boost::filesystem;

std::mutex slicePreprocessor;

namespace Slicer {
	Slicer::Slicer(FILE * c) :
		components(0),
		cpp(c),
		classNo(0)
	{
	}

	void
	Slicer::defineConversions(Slice::DataMemberPtr dm) const
	{
		if (!cpp) return;

		auto type = dm->type();
		auto c = Slice::ContainedPtr::dynamicCast(dm->container());
		auto conversions = getAllConversions(dm);
		for (const auto & conversion : conversions) {
			fprintbf(cpp, "%s %s(const %s &);\n",
					conversion.ExchangeType,
					conversion.ConvertToExchangeFunc,
					Slice::typeToString(type));
			fprintbf(cpp, "%s %s(const %s &);\n\n",
					Slice::typeToString(type),
					conversion.ConvertToModelFunc,
					conversion.ExchangeType);
		}
		if (!conversions.empty()) {
			fprintbf(cpp, "template<>\nvoid\n");
			fprintbf(cpp, "ModelPartForConverted< %s, %s, &%s >::SetValue(ValueSourcePtr vsp)\n",
					Slice::typeToString(type),
					c->scoped(),
					dm->scoped());
			fprintbf(cpp, "{\n");

			for (const auto & conversion : conversions) {
				fprintbf(cpp, "\tif (auto vspt = dynamic_cast<TValueSource< %s > *>(vsp.get())) {\n",
						conversion.ExchangeType);
				fprintbf(cpp, "\t\t%s tmp;\n",
						conversion.ExchangeType);
				fprintbf(cpp, "\t\tvspt->set(tmp);\n");
				fprintbf(cpp, "\t\tMember = %s(tmp);\n",
						conversion.ConvertToModelFunc);
				fprintbf(cpp, "\t\treturn;\n");
				fprintbf(cpp, "\t}\n");
			}
			fprintbf(cpp, "}\n\n");

			fprintbf(cpp, "template<>\nvoid\n");
			fprintbf(cpp, "ModelPartForConverted< %s, %s, &%s >::GetValue(ValueTargetPtr vtp)\n",
					Slice::typeToString(type),
					c->scoped(),
					dm->scoped());
			fprintbf(cpp, "{\n");

			for (const auto & conversion : conversions) {
				fprintbf(cpp, "\tif (auto vtpt = dynamic_cast<TValueTarget< %s > *>(vtp.get())) {\n",
						conversion.ExchangeType);
				fprintbf(cpp, "\t\tvtpt->get(%s(Member));\n",
						conversion.ConvertToExchangeFunc);
				fprintbf(cpp, "\t\treturn;\n");
				fprintbf(cpp, "\t}\n");
			}
			fprintbf(cpp, "}\n\n");
		}
	}

	bool
	Slicer::visitUnitStart(const Slice::UnitPtr & u)
	{
		fs::path topLevelFile(u->topLevelFile());
		if (!cpp) return true;

		fprintbf(cpp, "// Begin Slicer code\n\n");
		fprintbf(cpp, "#include <%s>\n\n", fs::change_extension(topLevelFile.filename(), ".h").string());
		fprintbf(cpp, "#include <slicer/modelParts.h>\n\n");
		fprintbf(cpp, "#define templateMODELPARTFOR(Type, ModelPart) \\\n");
		fprintbf(cpp, "template <> ModelPartPtr DLL_PUBLIC ModelPartFor(IceUtil::Optional<Type> & t) { return new ModelPartForOptional< ModelPart< Type > >(t); } \\\n");
		fprintbf(cpp, "template <> ModelPartPtr DLL_PUBLIC ModelPartFor(IceUtil::Optional<Type> * t) { return new ModelPartForOptional< ModelPart< Type > >(t); } \\\n");
		fprintbf(cpp, "template <> ModelPartPtr DLL_PUBLIC ModelPartFor(Type & t) { return new ModelPart< Type >(t); } \\\n");
		fprintbf(cpp, "template <> ModelPartPtr DLL_PUBLIC ModelPartFor(Type * t) { return new ModelPart< Type >(t); }\n\n");
		fprintbf(cpp, "namespace Slicer {\n");
		return true;
	}

	void
	Slicer::visitUnitEnd(const Slice::UnitPtr&)
	{
		if (!cpp) return;

		fprintbf(cpp, "}\n\n");
		fprintbf(cpp, "// End Slicer code\n\n");
	}

	bool
	Slicer::visitModuleStart(const Slice::ModulePtr & m)
	{
		if (!cpp) return true;

		fprintbf(cpp, "// Begin module %s\n\n", m->name());
		for (const auto & c : m->structs()) {
			for (const auto & dm : c->dataMembers()) {
				defineConversions(dm);
			}
		}
		for (const auto & c : m->classes()) {
			for (const auto & dm : c->dataMembers()) {
				defineConversions(dm);
			}
		}
		return true;
	}


	void
	Slicer::defineRootName(const std::string & type, const std::string & name) const
	{
		fprintbf(cpp, "template<>\n");
		fprintbf(cpp, "DLL_PUBLIC std::string ModelPartForRoot< %s >::rootName(\"%s\");\n\n",
				type, name);
		fprintbf(cpp, "template<>\n");
		fprintbf(cpp, "DLL_PUBLIC std::string ModelPartForRoot< IceUtil::Optional< %s > >::rootName(\"Optional%s\");\n\n",
				type, name);
	}

	bool
	Slicer::visitClassDefStart(const Slice::ClassDefPtr & c)
	{
		if (c->isInterface()) { return false; }
		if (c->hasMetaData("slicer:ignore")) { return false; }

		components += 1;

		if (!cpp) return true;

		auto decl = c->declaration();
		fprintbf(cpp, "// Class %s\n", c->name());
		visitComplexDataMembers(decl, c->allDataMembers());

		fprintbf(cpp, "template<>\n");
		auto typeId = metaDataValue("slicer:typeid:", c->getMetaData());
		fprintbf(cpp, "DLL_PUBLIC std::string ModelPartForClass< %s >::typeIdProperty(\"%s\");\n\n",
				typeToString(decl),
				typeId ? *typeId : "slicer-typeid");

		auto name = metaDataValue("slicer:root:", c->getMetaData());
		defineRootName(typeToString(decl), name ? *name : c->name());

		auto typeName = metaDataValue("slicer:typename:", c->getMetaData());
		fprintbf(cpp, "static void registerClass_%u() __attribute__ ((constructor(210)));\n", classNo);
		fprintbf(cpp, "static void registerClass_%u()\n{\n", classNo);
		fprintbf(cpp, "\tSlicer::classRefMap()->insert({ \"%s\", [](void * p){ return new ModelPartForClass< %s >(*static_cast< %s *>(p)); } });\n",
				c->scoped(),
				typeToString(decl),
				typeToString(decl));
		if (typeName) {
			fprintbf(cpp, "\tSlicer::classNameMap()->insert({ \"%s\", \"%s\" });\n",
					c->scoped(),
					*typeName);
		}
		fprintbf(cpp, "}\n\n");
		fprintbf(cpp, "static void unregisterClass_%u() __attribute__ ((destructor(210)));\n", classNo);
		fprintbf(cpp, "static void unregisterClass_%u()\n{\n", classNo);
		fprintbf(cpp, "\tSlicer::classRefMap()->erase(\"%s\");\n",
				c->scoped());
		if (typeName) {
			fprintbf(cpp, "\tSlicer::classNameMap()->left.erase(\"%s\");\n",
					c->scoped());
		}
		fprintbf(cpp, "}\n\n");

		fprintbf(cpp, "template<>\nDLL_PUBLIC TypeId\nModelPartForClass< %s >::GetTypeId() const\n{\n",
				typeToString(decl));
		fprintbf(cpp, "\tauto id = ModelObject->ice_id();\n");
		fprintbf(cpp, "\treturn (id == \"%s\") ? TypeId() : ModelPart::ToExchangeTypeName(id);\n}\n\n",
				c->scoped());

		fprintbf(cpp, "template<>\nDLL_PUBLIC Metadata ModelPartForComplex< %s >::metadata ",
				c->scoped());
		copyMetadata(c->getMetaData());

		fprintbf(cpp, "templateMODELPARTFOR(::IceInternal::Handle< %s >, ModelPartForClass);\n\n",
				c->scoped());

		classNo += 1;

		return true;
	}

	bool
	Slicer::visitStructStart(const Slice::StructPtr & c)
	{
		if (c->hasMetaData("slicer:ignore")) { return false; }

		components += 1;

		if (!cpp) return true;

		fprintbf(cpp, "// Struct %s\n", c->name());
		visitComplexDataMembers(c, c->dataMembers());

		auto name = metaDataValue("slicer:root:", c->getMetaData());
		defineRootName(c->scoped(), name ? *name : c->name());

		fprintbf(cpp, "template<>\nDLL_PUBLIC Metadata ModelPartForComplex< %s >::metadata ",
				c->scoped());
		copyMetadata(c->getMetaData());

		fprintbf(cpp, "templateMODELPARTFOR(%s, ModelPartForStruct);\n\n",
				c->scoped());

		return true;
	}

	void
	Slicer::visitComplexDataMembers(Slice::ConstructedPtr it, const Slice::DataMemberList & dataMembers) const
	{
		if (!cpp) return;

		fprintbf(cpp, "template<>\n");
		fprintbf(cpp, "DLL_PUBLIC ModelPartForComplex< %s >::Hooks ",
				it->scoped());
		fprintbf(cpp, "ModelPartForComplex< %s >::hooks {\n",
				it->scoped());
		for (const auto & dm : dataMembers) {
			auto c = Slice::ContainedPtr::dynamicCast(dm->container());
			auto t = Slice::TypePtr::dynamicCast(dm->container());
			if (!t) {
				t = Slice::ClassDefPtr::dynamicCast(dm->container())->declaration();
			}
			auto name = metaDataValue("slicer:name:", dm->getMetaData());
			auto conversions = getAllConversions(dm);
			fprintbf(cpp, "\t\tnew ");
			auto type = dm->type();
			createNewModelPartPtrFor(t);
			fprintbf(cpp, "< %s >::Hook< %s",
					typeToString(it),
					Slice::typeToString(type, dm->optional()));
			fprintbf(cpp, ", %s, &%s, ",
					boost::algorithm::trim_right_copy_if(dm->container()->thisScope(), ispunct),
					dm->scoped());
			if (dm->optional()) {
				fprintbf(cpp, "ModelPartForOptional< ");
			}
			if (!conversions.empty()) {
				fprintbf(cpp, "ModelPartForConverted< %s, %s, &%s >",
						Slice::typeToString(type),
						boost::algorithm::trim_right_copy_if(dm->container()->thisScope(), ispunct),
						dm->scoped());
			}
			else {
				createNewModelPartPtrFor(type);
				fprintbf(cpp, "< %s >",
						Slice::typeToString(type));
			}
			if (dm->optional()) {
				fprintbf(cpp, " > ");
			}
			fprintbf(cpp, " >(\"%s\"),\n",
					name ? *name : dm->name());
		}
		fprintbf(cpp, "\t};\n\n");

		for (const auto & dm : dataMembers) {
			auto c = Slice::ContainedPtr::dynamicCast(dm->container());
			auto t = Slice::TypePtr::dynamicCast(dm->container());
			if (!t) {
				t = Slice::ClassDefPtr::dynamicCast(dm->container())->declaration();
			}
			auto type = dm->type();
			fprintbf(cpp, "template<>\ntemplate<>\nMetadata\n");
			createNewModelPartPtrFor(t);
			fprintbf(cpp, "< %s >::HookMetadata< %s",
					typeToString(it),
					Slice::typeToString(type, dm->optional()));
			fprintbf(cpp, ", %s, &%s >::metadata ",
					boost::algorithm::trim_right_copy_if(dm->container()->thisScope(), ispunct),
					dm->scoped());
			copyMetadata(dm->getMetaData());
		}
	}

	void
	Slicer::visitEnum(const Slice::EnumPtr & e)
	{
		if (e->hasMetaData("slicer:ignore")) { return; }

		components += 1;

		if (!cpp) return;

		fprintbf(cpp, "// Enumeration %s\n", e->name());
		fprintbf(cpp, "template<>\nMetadata ModelPartForEnum< %s >::metadata ",
				e->scoped());
		copyMetadata(e->getMetaData());

		fprintbf(cpp, "template<>\nModelPartForEnum< %s >::Enumerations\nModelPartForEnum< %s >::enumerations([]() -> ModelPartForEnum< %s >::Enumerations\n",
				e->scoped(),
				e->scoped(),
				e->scoped());
		fprintbf(cpp, "{\n\tModelPartForEnum< %s >::Enumerations e;\n",
				e->scoped());
		for (const auto & ee : e->getEnumerators()) {
			fprintbf(cpp, "\te.insert( { %s, \"%s\" } );\n", ee->scoped(), ee->name());
		}
		fprintbf(cpp, "\treturn e;\n}());\n\n");

		fprintbf(cpp, "template<>\nvoid ModelPartForEnum< %s >::SetValue(ValueSourcePtr s) {\n\
	std::string val;\n\
	s->set(val);\n\
	auto i = enumerations.right.find(val);\n\
	if (i == enumerations.right.end()) throw InvalidEnumerationValue(val, \"%s\");\n\
	modelPart = i->second;\n\
}\n\n",
				e->scoped(),
				e->scoped());
		fprintbf(cpp, "template<>\nvoid ModelPartForEnum< %s >::GetValue(ValueTargetPtr s) {\n\
	auto i = enumerations.left.find(modelPart);\n\
	if (i == enumerations.left.end()) throw InvalidEnumerationValue((::Ice::Int)modelPart, \"%s\");\n\
	s->get(i->second);\n\
}\n\n",
				e->scoped(),
				e->scoped());

		auto name = metaDataValue("slicer:root:", e->getMetaData());
		defineRootName(e->scoped(), name ? *name : e->name());

		fprintbf(cpp, "templateMODELPARTFOR(%s, ModelPartForEnum);\n\n",
				e->scoped());
	}

	void
	Slicer::visitSequence(const Slice::SequencePtr & s)
	{
		if (s->hasMetaData("slicer:ignore")) { return; }

		components += 1;

		if (!cpp) return;

		fprintbf(cpp, "// Sequence %s\n", s->name());
		fprintbf(cpp, "template<>\n");
		fprintbf(cpp, "DLL_PUBLIC ChildRefPtr ModelPartForSequence< %s >::GetChildRef(const std::string & name, const HookFilter & flt)\n{\n",
				s->scoped());
		auto iname = metaDataValue("slicer:item:", s->getMetaData());
		if (iname) {
			fprintbf(cpp, "\tif (!name.empty() && name != \"%s\") { throw IncorrectElementName(); }\n",
					*iname);
		}
		else {
			fprintbf(cpp, "\t(void)name;\n");
		}
		fprintbf(cpp, "\treturn GetAnonChildRef(flt);\n}\n\n");

		fprintbf(cpp, "template<>\n");
		fprintbf(cpp, "DLL_PUBLIC ModelPartPtr\n");
		fprintbf(cpp, "ModelPartForSequence< %s >::elementModelPart(typename %s::value_type & e) const {\n",
				s->scoped(),
				s->scoped());
		fprintbf(cpp, "\treturn ModelPartFor(e);\n}\n\n");

		fprintbf(cpp, "template<>\n");
		auto ename = metaDataValue("slicer:element:", s->getMetaData());
		fprintbf(cpp, "DLL_PUBLIC std::string ModelPartForSequence< %s >::elementName(\"%s\");\n\n",
				s->scoped(),
				ename ? *ename : "element");

		auto name = metaDataValue("slicer:root:", s->getMetaData());
		defineRootName(s->scoped(), name ? *name : s->name());

		fprintbf(cpp, "template<>\nDLL_PUBLIC Metadata ModelPartForSequence< %s >::metadata ",
				s->scoped());
		copyMetadata(s->getMetaData());

		fprintbf(cpp, "templateMODELPARTFOR(%s, ModelPartForSequence);\n\n",
				s->scoped());
	}

	void
	Slicer::visitDictionary(const Slice::DictionaryPtr & d)
	{
		if (d->hasMetaData("slicer:ignore")) { return; }

		components += 1;

		if (!cpp) return;

		fprintbf(cpp, "// Dictionary %s\n", d->name());
		auto iname = metaDataValue("slicer:item:", d->getMetaData());
		fprintbf(cpp, "template<>\n");
		fprintbf(cpp, "DLL_PUBLIC std::string ModelPartForDictionary< %s >::pairName(\"%s\");\n\n",
				d->scoped(),
				iname ? *iname : "element");

		fprintbf(cpp, "template<>\n");
		fprintbf(cpp, "DLL_PUBLIC ModelPartForComplex< ModelPartForDictionaryElement< %s > >::Hooks ",
				d->scoped());
		fprintbf(cpp, "ModelPartForComplex< ModelPartForDictionaryElement< %s > >::hooks {\n",
				d->scoped());
		auto kname = metaDataValue("slicer:key:", d->getMetaData());
		auto vname = metaDataValue("slicer:value:", d->getMetaData());
		fprintbf(cpp, "\t\t");
		auto ktype = d->keyType();
		fprintbf(cpp, "new ModelPartForDictionaryElement< %s >::Hook< %s*, ModelPartForDictionaryElement< %s >, &ModelPartForDictionaryElement< %s >::key, ",
				d->scoped(),
				Slice::typeToString(ktype),
				d->scoped(),
				d->scoped());
		createNewModelPartPtrFor(ktype);
		fprintbf(cpp, "< %s > >(\"%s\"),\n\t\t",
				Slice::typeToString(ktype),
				kname ? *kname : "key");
		auto vtype = d->valueType();
		fprintbf(cpp, "new ModelPartForDictionaryElement< %s >::Hook< %s*, ModelPartForDictionaryElement< %s >, &ModelPartForDictionaryElement< %s >::value, ",
				d->scoped(),
				Slice::typeToString(vtype),
				d->scoped(),
				d->scoped());
		createNewModelPartPtrFor(vtype);
		fprintbf(cpp, "< %s > >(\"%s\"),\n",
				Slice::typeToString(vtype),
				vname ? *vname : "value");
		fprintbf(cpp, "\t};\n");
		fprintbf(cpp, "\n");

		auto name = metaDataValue("slicer:root:", d->getMetaData());
		defineRootName(d->scoped(), name ? *name : d->name());

		fprintbf(cpp, "template<>\nDLL_PUBLIC Metadata ModelPartForDictionary< %s >::metadata ",
				d->scoped());
		copyMetadata(d->getMetaData());

		fprintbf(cpp, "template<>\nDLL_PUBLIC Metadata ModelPartForComplex<ModelPartForDictionaryElement< %s > >::metadata ",
				d->scoped());
		copyMetadata(d->getMetaData());

		fprintbf(cpp, "template<>\ntemplate<>\nMetadata\nModelPartForDictionaryElement< %s >::HookMetadata< %s*, ModelPartForDictionaryElement< %s >, &ModelPartForDictionaryElement< %s >::key >::metadata { };\n\n",
				d->scoped(),
				Slice::typeToString(ktype),
				d->scoped(),
				d->scoped());
		fprintbf(cpp, "template<>\ntemplate<>\nMetadata\nModelPartForDictionaryElement< %s >::HookMetadata< %s*, ModelPartForDictionaryElement< %s >, &ModelPartForDictionaryElement< %s >::value >::metadata { };\n\n",
				d->scoped(),
				Slice::typeToString(vtype),
				d->scoped(),
				d->scoped());

		fprintbf(cpp, "templateMODELPARTFOR(%s, ModelPartForDictionary);\n\n",
				d->scoped());
	}

	void
	Slicer::visitModuleEnd(const Slice::ModulePtr & m)
	{
		if (cpp) {
			fprintbf(cpp, "// End module %s\n\n", m->name());
		}
	}

	void
	Slicer::createNewModelPartPtrFor(const Slice::TypePtr & type) const
	{
		if (auto builtin = Slice::BuiltinPtr::dynamicCast(type)) {
			fprintbf(cpp, "ModelPartForSimple");
		}
		else if (auto complexClass = Slice::ClassDeclPtr::dynamicCast(type)) {
			fprintbf(cpp, "ModelPartForClass");
		}
		else if (auto complexStruct = Slice::StructPtr::dynamicCast(type)) {
			fprintbf(cpp, "ModelPartForStruct");
		}
		else if (auto sequence = Slice::SequencePtr::dynamicCast(type)) {
			fprintbf(cpp, "ModelPartForSequence");
		}
		else if (auto dictionary = Slice::DictionaryPtr::dynamicCast(type)) {
			fprintbf(cpp, "ModelPartForDictionary");
		}
		else if (auto enumeration = Slice::EnumPtr::dynamicCast(type)) {
			fprintbf(cpp, "ModelPartForEnum");
		}
	}

	void
	Slicer::copyMetadata(const std::list<std::string> & metadata) const
	{
		fprintbf(cpp, "{\n");
		for (const auto & md : metadata) {
			if (boost::algorithm::starts_with(md, "slicer:")) {
				fprintbf(cpp, "\t\"%s\",\n", md.c_str() + 7);
			}
		}
		fprintbf(cpp, "};\n\n");
	}

	Slicer::Conversions
	Slicer::getAllConversions(Slice::DataMemberPtr dm)
	{
		auto conversions = getConversions(dm->getMetaData());
		auto typec = Slice::ContainedPtr::dynamicCast(dm->type());
		if (typec) {
			auto typeConversions = getConversions(typec->getMetaData());
			std::copy(typeConversions.begin(), typeConversions.end(), std::back_inserter(conversions));
		}
		return conversions;
	}

	Slicer::Conversions
	Slicer::getConversions(const std::list<std::string> & dm)
	{
		Conversions rtn;
		auto conversions = metaDataValues("slicer:conversion:", dm);
		for (const auto & conversion : conversions) {
			auto split = metaDataSplit(conversion);
			if (split.size() < 3) {
				throw std::runtime_error("conversion needs at least 3 parts type:toModelFunc:toExchangeFunc[:options]");
			}
			for (auto & pi : {0, 1, 2}) {
				boost::algorithm::replace_all(split[pi], ".", "::");
			}
			rtn.push_back(split);
		}
		return rtn;
	}

	unsigned int
	Slicer::Components() const
	{
		return components;
	}

	unsigned int
	Slicer::Apply(const boost::filesystem::path & ice, const boost::filesystem::path & cpp)
	{
		return Apply(ice, cpp, {});
	}

	unsigned int
	Slicer::Apply(const boost::filesystem::path & ice, const boost::filesystem::path & cpp, const Args & args)
	{
		FilePtr cppfile(fopen(cpp.string(), "a"), fclose);
		if (!cppfile) {
			throw std::runtime_error("failed to open code file");
		}

		return Apply(ice, cppfile.get(), args);
	}

	unsigned int
	Slicer::Apply(const boost::filesystem::path & ice, FILE * cpp)
	{
		return Apply(ice, cpp, {});
	}

	unsigned int
	Slicer::Apply(const boost::filesystem::path & ice, FILE * cpp, const Args & args)
	{
		std::lock_guard<std::mutex> lock(slicePreprocessor);
		Slice::PreprocessorPtr icecpp = Slice::Preprocessor::create("slicer", ice.string(), args);
		FILE * cppHandle = icecpp->preprocess(false);

		if (cppHandle == NULL) {
			throw std::runtime_error("preprocess failed");
		}

		Slice::UnitPtr u = Slice::Unit::createUnit(false, false, false, false);

		int parseStatus = u->parse(ice.string(), cppHandle, false);

		if (!icecpp->close()) {
			throw std::runtime_error("preprocess close failed");
		}

		if (parseStatus == EXIT_FAILURE) {
			throw std::runtime_error("unit parse failed");
		}

		Slicer s(cpp);
		u->visit(&s, false);

		u->destroy();

		return s.Components();
	}

	Slicer::ConversionSpec::ConversionSpec(const Slicer::Args & s) :
		ExchangeType(s[0]),
		ConvertToModelFunc(s[1]),
		ConvertToExchangeFunc(s[2])
	{
		if (s.size() >= 4) {
			boost::algorithm::split(Options, s[3], boost::algorithm::is_any_of(","), boost::algorithm::token_compress_off);
		}
	}
};

