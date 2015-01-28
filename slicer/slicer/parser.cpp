#include "parser.h"
#include "metadata.h"
#include <Slice/Parser.h>
#include <Slice/Preprocessor.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <Slice/CPlusPlusUtil.h>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/convenience.hpp>
#include <mutex>

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
			fprintf(cpp, "%s %s(const %s &);\n",
					conversion.ExchangeType.c_str(),
					conversion.ConvertToExchangeFunc.c_str(),
					Slice::typeToString(type).c_str());
			fprintf(cpp, "%s %s(const %s &);\n\n",
					Slice::typeToString(type).c_str(),
					conversion.ConvertToModelFunc.c_str(),
					conversion.ExchangeType.c_str());
		}
		if (!conversions.empty()) {
			fprintf(cpp, "template<>\nvoid\n");
			fprintf(cpp, "ModelPartForConverted< %s, %s, &%s >::SetValue(ValueSourcePtr vsp)\n",
					Slice::typeToString(type).c_str(),
					c->scoped().c_str(),
					dm->scoped().c_str());
			fprintf(cpp, "{\n");

			for (const auto & conversion : conversions) {
				fprintf(cpp, "\tif (auto vspt = dynamic_cast<TValueSource< %s > *>(vsp.get())) {\n",
						conversion.ExchangeType.c_str());
				fprintf(cpp, "\t\t%s tmp;\n",
						conversion.ExchangeType.c_str());
				fprintf(cpp, "\t\tvspt->set(tmp);\n");
				fprintf(cpp, "\t\tMember = %s(tmp);\n",
						conversion.ConvertToModelFunc.c_str());
				fprintf(cpp, "\t\treturn;\n");
				fprintf(cpp, "\t}\n");
			}
			fprintf(cpp, "}\n\n");

			fprintf(cpp, "template<>\nvoid\n");
			fprintf(cpp, "ModelPartForConverted< %s, %s, &%s >::GetValue(ValueTargetPtr vtp)\n",
					Slice::typeToString(type).c_str(),
					c->scoped().c_str(),
					dm->scoped().c_str());
			fprintf(cpp, "{\n");

			for (const auto & conversion : conversions) {
				fprintf(cpp, "\tif (auto vtpt = dynamic_cast<TValueTarget< %s > *>(vtp.get())) {\n",
						conversion.ExchangeType.c_str());
				fprintf(cpp, "\t\tvtpt->get(%s(Member));\n",
						conversion.ConvertToExchangeFunc.c_str());
				fprintf(cpp, "\t\treturn;\n");
				fprintf(cpp, "\t}\n");
			}
			fprintf(cpp, "}\n\n");
		}
	}

	bool
	Slicer::visitUnitStart(const Slice::UnitPtr & u)
	{
		fs::path topLevelFile(u->topLevelFile());
		if (!cpp) return true;

		fprintf(cpp, "// Begin Slicer code\n\n");
		fprintf(cpp, "#include <%s>\n\n", fs::change_extension(topLevelFile.filename(), ".h").string().c_str());
		fprintf(cpp, "#include <slicer/modelParts.h>\n\n");
		fprintf(cpp, "#define templateMODELPARTFOR(Type, ModelPart) \\\n");
		fprintf(cpp, "template <> ModelPartPtr ModelPartFor(Type & t) { return new ModelPart< Type >(t); } \\\n");
		fprintf(cpp, "template <> ModelPartPtr ModelPartFor(Type * t) { return new ModelPart< Type >(t); }\n\n");
		fprintf(cpp, "namespace Slicer {\n");
		return true;
	}

	void
	Slicer::visitUnitEnd(const Slice::UnitPtr&)
	{
		if (!cpp) return;

		fprintf(cpp, "}\n\n");
		fprintf(cpp, "// End Slicer code\n\n");
	}

	bool
	Slicer::visitModuleStart(const Slice::ModulePtr & m)
	{
		if (!cpp) return true;

		fprintf(cpp, "// Begin module %s\n\n", m->name().c_str());
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
		fprintf(cpp, "template<>\n");
		fprintf(cpp, "std::string ModelPartForRoot< %s >::rootName(\"%s\");\n\n",
				type.c_str(), name.c_str());
	}

	bool
	Slicer::visitClassDefStart(const Slice::ClassDefPtr & c)
	{
		if (c->isInterface()) { return false; }
		if (c->hasMetaData("slicer:ignore")) { return false; }

		components += 1;

		if (!cpp) return true;

		auto decl = c->declaration();
		fprintf(cpp, "// Class %s\n", c->name().c_str());
		visitComplexDataMembers(decl, c->allDataMembers());
		
		fprintf(cpp, "template<>\n");
		auto typeId = metaDataValue("slicer:typeid:", c->getMetaData());
		fprintf(cpp, "std::string ModelPartForClass< %s >::typeIdProperty(\"%s\");\n\n",
				typeToString(decl).c_str(),
				typeId ? typeId->c_str() : "slicer-typeid");

		auto name = metaDataValue("slicer:root:", c->getMetaData());
		defineRootName(typeToString(decl), name ? *name : c->name());

		auto typeName = metaDataValue("slicer:typename:", c->getMetaData());
		fprintf(cpp, "static void registerClass_%u() __attribute__ ((constructor(210)));\n", classNo);
		fprintf(cpp, "static void registerClass_%u()\n{\n", classNo);
		fprintf(cpp, "\tSlicer::classRefMap()->insert({ \"%s\", [](void * p){ return new ModelPartForClass< %s >(*static_cast< %s *>(p)); } });\n",
				c->scoped().c_str(),
				typeToString(decl).c_str(),
				typeToString(decl).c_str());
		if (typeName) {
			fprintf(cpp, "\tSlicer::classNameMap()->insert({ \"%s\", \"%s\" });\n",
					c->scoped().c_str(),
					typeName->c_str());
		}
		fprintf(cpp, "}\n\n");
		fprintf(cpp, "static void unregisterClass_%u() __attribute__ ((destructor(210)));\n", classNo);
		fprintf(cpp, "static void unregisterClass_%u()\n{\n", classNo);
		fprintf(cpp, "\tSlicer::classRefMap()->erase(\"%s\");\n",
				c->scoped().c_str());
		if (typeName) {
			fprintf(cpp, "\tSlicer::classNameMap()->left.erase(\"%s\");\n",
					c->scoped().c_str());
		}
		fprintf(cpp, "}\n\n");

		fprintf(cpp, "template<>\nTypeId\nModelPartForClass< %s >::GetTypeId() const\n{\n",
				typeToString(decl).c_str());
		fprintf(cpp, "\tauto id = ModelObject->ice_id();\n");
		fprintf(cpp, "\treturn (id == \"%s\") ? TypeId() : ModelPart::ToExchangeTypeName(id);\n}\n\n",
				c->scoped().c_str());

		fprintf(cpp, "template<>\nMetadata ModelPartForComplex< %s >::metadata ",
				c->scoped().c_str());
		copyMetadata(c->getMetaData());

		fprintf(cpp, "templateMODELPARTFOR(::IceInternal::Handle< %s >, ModelPartForClass);\n\n",
				c->scoped().c_str());

		classNo += 1;

		return true;
	}

	bool
	Slicer::visitStructStart(const Slice::StructPtr & c)
	{
		if (c->hasMetaData("slicer:ignore")) { return false; }

		components += 1;

		if (!cpp) return true;

		fprintf(cpp, "// Struct %s\n", c->name().c_str());
		visitComplexDataMembers(c, c->dataMembers());

		auto name = metaDataValue("slicer:root:", c->getMetaData());
		defineRootName(c->scoped(), name ? *name : c->name());

		fprintf(cpp, "template<>\nMetadata ModelPartForComplex< %s >::metadata ",
				c->scoped().c_str());
		copyMetadata(c->getMetaData());

		fprintf(cpp, "templateMODELPARTFOR(%s, ModelPartForStruct);\n\n",
				c->scoped().c_str());

		return true;
	}

	void
	Slicer::visitComplexDataMembers(Slice::ConstructedPtr it, const Slice::DataMemberList & dataMembers) const
	{
		if (!cpp) return;

		fprintf(cpp, "template<>\n");
		fprintf(cpp, "ModelPartForComplex< %s >::Hooks ",
				it->scoped().c_str());
		fprintf(cpp, "ModelPartForComplex< %s >::hooks {\n",
				it->scoped().c_str());
		for (const auto & dm : dataMembers) {
			auto c = Slice::ContainedPtr::dynamicCast(dm->container());
			auto t = Slice::TypePtr::dynamicCast(dm->container());
			if (!t) {
				t = Slice::ClassDefPtr::dynamicCast(dm->container())->declaration();
			}
			auto name = metaDataValue("slicer:name:", dm->getMetaData());
			auto conversions = getAllConversions(dm);
			fprintf(cpp, "\t\tnew ");
			auto type = dm->type();
			createNewModelPartPtrFor(t);
			fprintf(cpp, "< %s >::Hook< %s",
					typeToString(it).c_str(),
					Slice::typeToString(type, dm->optional()).c_str());
			fprintf(cpp, ", %s, &%s, ",
					boost::algorithm::trim_right_copy_if(dm->container()->thisScope(), ispunct).c_str(),
					dm->scoped().c_str());
			if (dm->optional()) {
				fprintf(cpp, "ModelPartForOptional< ");
			}
			if (!conversions.empty()) {
				fprintf(cpp, "ModelPartForConverted< %s, %s, &%s >",
						Slice::typeToString(type).c_str(),
						boost::algorithm::trim_right_copy_if(dm->container()->thisScope(), ispunct).c_str(),
						dm->scoped().c_str());
			}
			else {
				createNewModelPartPtrFor(type);
				fprintf(cpp, "< %s >",
						Slice::typeToString(type).c_str());
			}
			if (dm->optional()) {
				fprintf(cpp, " > ");
			}
			fprintf(cpp, " >(\"%s\"),\n",
					name ? name->c_str() : dm->name().c_str());
		}
		fprintf(cpp, "\t};\n\n");

		for (const auto & dm : dataMembers) {
			auto c = Slice::ContainedPtr::dynamicCast(dm->container());
			auto t = Slice::TypePtr::dynamicCast(dm->container());
			if (!t) {
				t = Slice::ClassDefPtr::dynamicCast(dm->container())->declaration();
			}
			auto type = dm->type();
			fprintf(cpp, "template<>\ntemplate<>\nMetadata\n");
			createNewModelPartPtrFor(t);
			fprintf(cpp, "< %s >::HookMetadata< %s",
					typeToString(it).c_str(),
					Slice::typeToString(type, dm->optional()).c_str());
			fprintf(cpp, ", %s, &%s >::metadata ",
					boost::algorithm::trim_right_copy_if(dm->container()->thisScope(), ispunct).c_str(),
					dm->scoped().c_str());
			copyMetadata(dm->getMetaData());
		}
	}

	void
	Slicer::visitEnum(const Slice::EnumPtr & e)
	{
		if (e->hasMetaData("slicer:ignore")) { return; }

		components += 1;

		if (!cpp) return;

		fprintf(cpp, "// Enumeration %s\n", e->name().c_str());
		fprintf(cpp, "template<>\nMetadata ModelPartForEnum< %s >::metadata ",
				e->scoped().c_str());
		copyMetadata(e->getMetaData());

		fprintf(cpp, "template<>\nModelPartForEnum< %s >::Enumerations\nModelPartForEnum< %s >::enumerations([]() -> ModelPartForEnum< %s >::Enumerations\n",
				e->scoped().c_str(),
				e->scoped().c_str(),
				e->scoped().c_str());
		fprintf(cpp, "{\n\tModelPartForEnum< %s >::Enumerations e;\n",
				e->scoped().c_str());
		for (const auto & ee : e->getEnumerators()) {
			fprintf(cpp, "\te.insert( { %s, \"%s\" } );\n", ee->scoped().c_str(), ee->name().c_str());
			
		}
		fprintf(cpp, "\treturn e;\n}());\n\n");

		fprintf(cpp, "template<>\nvoid ModelPartForEnum< %s >::SetValue(ValueSourcePtr s) {\n\
	std::string val;\n\
	s->set(val);\n\
	auto i = enumerations.right.find(val);\n\
	if (i == enumerations.right.end()) throw InvalidEnumerationValue(val, \"%s\");\n\
	modelPart = i->second;\n\
}\n\n",
				e->scoped().c_str(),
				e->scoped().c_str());
		fprintf(cpp, "template<>\nvoid ModelPartForEnum< %s >::GetValue(ValueTargetPtr s) {\n\
	auto i = enumerations.left.find(modelPart);\n\
	if (i == enumerations.left.end()) throw InvalidEnumerationValue((::Ice::Int)modelPart, \"%s\");\n\
	s->get(i->second);\n\
}\n\n",
				e->scoped().c_str(),
				e->scoped().c_str());

		auto name = metaDataValue("slicer:root:", e->getMetaData());
		defineRootName(e->scoped(), name ? *name : e->name());

		fprintf(cpp, "templateMODELPARTFOR(%s, ModelPartForEnum);\n\n",
				e->scoped().c_str());
	}

	void
	Slicer::visitSequence(const Slice::SequencePtr & s)
	{
		if (s->hasMetaData("slicer:ignore")) { return; }

		components += 1;

		if (!cpp) return;

		fprintf(cpp, "// Sequence %s\n", s->name().c_str());
		fprintf(cpp, "template<>\n");
		fprintf(cpp, "ChildRefPtr ModelPartForSequence< %s >::GetChildRef(const std::string & name, const HookFilter & flt)\n{\n",
				s->scoped().c_str());
		auto iname = metaDataValue("slicer:item:", s->getMetaData());
		if (iname) {
			fprintf(cpp, "\tif (!name.empty() && name != \"%s\") { throw IncorrectElementName(); }\n",
					iname->c_str());
		}
		else {
			fprintf(cpp, "\t(void)name;\n");
		}
		fprintf(cpp, "\treturn GetAnonChildRef(flt);\n}\n\n");

		fprintf(cpp, "template<>\n");
		fprintf(cpp, "ModelPartPtr\n");
		fprintf(cpp, "ModelPartForSequence< %s >::elementModelPart(typename %s::value_type & e) const {\n",
				s->scoped().c_str(),
				s->scoped().c_str());
		fprintf(cpp, "\treturn ModelPartFor(e);\n}\n\n");
		
		fprintf(cpp, "template<>\n");
		auto ename = metaDataValue("slicer:element:", s->getMetaData());
		fprintf(cpp, "std::string ModelPartForSequence< %s >::elementName(\"%s\");\n\n",
				s->scoped().c_str(),
				ename ? ename->c_str() : "element");

		auto name = metaDataValue("slicer:root:", s->getMetaData());
		defineRootName(s->scoped(), name ? *name : s->name());

		fprintf(cpp, "template<>\nMetadata ModelPartForSequence< %s >::metadata ",
				s->scoped().c_str());
		copyMetadata(s->getMetaData());

		fprintf(cpp, "templateMODELPARTFOR(%s, ModelPartForSequence);\n\n",
				s->scoped().c_str());
	}

	void
	Slicer::visitDictionary(const Slice::DictionaryPtr & d)
	{
		if (d->hasMetaData("slicer:ignore")) { return; }

		components += 1;

		if (!cpp) return;

		fprintf(cpp, "// Dictionary %s\n", d->name().c_str());
		auto iname = metaDataValue("slicer:item:", d->getMetaData());
		fprintf(cpp, "template<>\n");
		fprintf(cpp, "std::string ModelPartForDictionary< %s >::pairName(\"%s\");\n\n",
				d->scoped().c_str(),
				iname ? iname->c_str() : "element");

		fprintf(cpp, "template<>\n");
		fprintf(cpp, "ModelPartForComplex< ModelPartForDictionaryElement< %s > >::Hooks ",
				d->scoped().c_str());
		fprintf(cpp, "ModelPartForComplex< ModelPartForDictionaryElement< %s > >::hooks {\n",
				d->scoped().c_str());
		auto kname = metaDataValue("slicer:key:", d->getMetaData());
		auto vname = metaDataValue("slicer:value:", d->getMetaData());
		fprintf(cpp, "\t\t");
		auto ktype = d->keyType();
		fprintf(cpp, "new ModelPartForDictionaryElement< %s >::Hook< %s*, ModelPartForDictionaryElement< %s >, &ModelPartForDictionaryElement< %s >::key, ",
				d->scoped().c_str(),
				Slice::typeToString(ktype).c_str(),
				d->scoped().c_str(),
				d->scoped().c_str());
		createNewModelPartPtrFor(ktype);
		fprintf(cpp, "< %s > >(\"%s\"),\n\t\t",
				Slice::typeToString(ktype).c_str(),
				kname ? kname->c_str() : "key");
		auto vtype = d->valueType();
		fprintf(cpp, "new ModelPartForDictionaryElement< %s >::Hook< %s*, ModelPartForDictionaryElement< %s >, &ModelPartForDictionaryElement< %s >::value, ",
				d->scoped().c_str(),
				Slice::typeToString(vtype).c_str(),
				d->scoped().c_str(),
				d->scoped().c_str());
		createNewModelPartPtrFor(vtype);
		fprintf(cpp, "< %s > >(\"%s\"),\n",
				Slice::typeToString(vtype).c_str(),
				vname ? vname->c_str() : "value");
		fprintf(cpp, "\t};\n");
		fprintf(cpp, "\n");

		fprintf(cpp, "template<>\nMetadata ModelPartForDictionary< %s >::metadata ",
				d->scoped().c_str());
		copyMetadata(d->getMetaData());

		fprintf(cpp, "template<>\nMetadata ModelPartForComplex<ModelPartForDictionaryElement< %s > >::metadata ",
				d->scoped().c_str());
		copyMetadata(d->getMetaData());

		fprintf(cpp, "template<>\ntemplate<>\nMetadata\nModelPartForDictionaryElement< %s >::HookMetadata< %s*, ModelPartForDictionaryElement< %s >, &ModelPartForDictionaryElement< %s >::key >::metadata { };\n\n",
				d->scoped().c_str(),
				Slice::typeToString(ktype).c_str(),
				d->scoped().c_str(),
				d->scoped().c_str());
		fprintf(cpp, "template<>\ntemplate<>\nMetadata\nModelPartForDictionaryElement< %s >::HookMetadata< %s*, ModelPartForDictionaryElement< %s >, &ModelPartForDictionaryElement< %s >::value >::metadata { };\n\n",
				d->scoped().c_str(),
				Slice::typeToString(vtype).c_str(),
				d->scoped().c_str(),
				d->scoped().c_str());

		fprintf(cpp, "templateMODELPARTFOR(%s, ModelPartForDictionary);\n\n",
				d->scoped().c_str());
	}

	void
	Slicer::visitModuleEnd(const Slice::ModulePtr & m)
	{
		if (cpp) {
			fprintf(cpp, "// End module %s\n\n", m->name().c_str());
		}
	}

	void
	Slicer::createNewModelPartPtrFor(const Slice::TypePtr & type) const
	{
		if (auto builtin = Slice::BuiltinPtr::dynamicCast(type)) {
			fprintf(cpp, "ModelPartForSimple");
		}
		else if (auto complexClass = Slice::ClassDeclPtr::dynamicCast(type)) {
			fprintf(cpp, "ModelPartForClass");
		}
		else if (auto complexStruct = Slice::StructPtr::dynamicCast(type)) {
			fprintf(cpp, "ModelPartForStruct");
		}
		else if (auto sequence = Slice::SequencePtr::dynamicCast(type)) {
			fprintf(cpp, "ModelPartForSequence");
		}
		else if (auto dictionary = Slice::DictionaryPtr::dynamicCast(type)) {
			fprintf(cpp, "ModelPartForDictionary");
		}
		else if (auto enumeration = Slice::EnumPtr::dynamicCast(type)) {
			fprintf(cpp, "ModelPartForEnum");
		}
	}

	void
	Slicer::copyMetadata(const std::list<std::string> & metadata) const
	{
		fprintf(cpp, "{\n");
		for (const auto & md : metadata) {
			if (boost::algorithm::starts_with(md, "slicer:")) {
				fprintf(cpp, "\t\"%.*s\",\n", (int)md.length() - 7, md.c_str() + 7);
			}
		}
		fprintf(cpp, "};\n\n");
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
			if (split.size() != 3) {
				throw std::runtime_error("conversion needs 3 parts type:toModelFunc:toExchangeFunc");
			}
			for (auto & p : split) {
				boost::algorithm::replace_all(p, ".", "::");
			}
			rtn.push_back(ConversionSpec({split[0], split[1], split[2]}));
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
		FilePtr cppfile(fopen(cpp.string().c_str(), "a"), fclose);
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
};

