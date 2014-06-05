#include "parser.h"
#include <Slice/Parser.h>
#include <Slice/Preprocessor.h>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <Slice/CPlusPlusUtil.h>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/convenience.hpp>

namespace fs = boost::filesystem;

namespace Slicer {
	Slicer::Slicer(FILE * c) :
		cpp(c)
	{
	}

	bool
	Slicer::visitUnitStart(const Slice::UnitPtr & u)
	{
		fs::path topLevelFile(u->topLevelFile());

		fprintf(cpp, "// Begin Slicer code\n\n");
		fprintf(cpp, "#include <%s>\n\n", fs::change_extension(topLevelFile.filename(), ".h").string().c_str());
		fprintf(cpp, "#include <slicer/modelParts.h>\n\n");
		fprintf(cpp, "namespace Slicer {\n");
		return true;
	}

	void
	Slicer::visitUnitEnd(const Slice::UnitPtr&)
	{
		fprintf(cpp, "}\n\n");
		fprintf(cpp, "// End Slicer code\n\n");
	}

	bool
	Slicer::visitModuleStart(const Slice::ModulePtr & m)
	{
		fprintf(cpp, "// Begin module %s\n\n", m->name().c_str());
		modules.push_back(m);
		return true;
	}

	bool
	Slicer::visitClassDefStart(const Slice::ClassDefPtr & c)
	{
		if (c->hasMetaData("slicer:ignore")) { return false; }

		fprintf(cpp, "// Class %s\n", c->name().c_str());
		fprintf(cpp, "template<>\n");
		fprintf(cpp, "ModelPartForComplex< %s::%s >::Hooks ",
				modulePath().c_str(), c->name().c_str());
		fprintf(cpp, "ModelPartForComplex< %s::%s >::hooks {\n",
				modulePath().c_str(), c->name().c_str());
		BOOST_FOREACH (const auto & dm, c->allDataMembers()) {
			auto name = metaDataValue("slicer:name:", dm->getMetaData());
			fprintf(cpp, "\t\t{ \"%s\", ",
					name ? name->c_str() : dm->name().c_str());
			auto type = dm->type();
			fprintf(cpp, "new ModelPartForClass< %s::%sPtr >::Hook< ",
				modulePath().c_str(), c->name().c_str());
			if (dm->optional()) {
				fprintf(cpp, "::IceUtil::Optional< %s >",
						Slice::typeToString(type).c_str());
			}
			else {
				fprintf(cpp, "%s",
						Slice::typeToString(type).c_str());
			}
			fprintf(cpp, ", &%s::%s::%s, ",
				modulePath().c_str(), c->name().c_str(), dm->name().c_str());
			if (dm->optional()) {
				fprintf(cpp, "ModelPartForOptional< ");
			}
			createNewModelPartPtrFor(type);
			fprintf(cpp, "< %s",
					Slice::typeToString(type).c_str());
			if (dm->optional()) {
				fprintf(cpp, " > ");
			}
			fprintf(cpp, " > >() },\n");
		}
		fprintf(cpp, "\t};\n");
		
		fprintf(cpp, "template<>\n");
		auto name = metaDataValue("slicer:root:", c->getMetaData());
		fprintf(cpp, "std::string ModelPartForClassRoot< %s::%sPtr >::rootName(\"%s\");\n\n",
				modulePath().c_str(), c->name().c_str(),
				name ? name->c_str() : c->name().c_str());
		return true;
	}

	bool
	Slicer::visitStructStart(const Slice::StructPtr & c)
	{
		if (c->hasMetaData("slicer:ignore")) { return false; }

		fprintf(cpp, "// Struct %s\n", c->name().c_str());
		fprintf(cpp, "template<>\n");
		fprintf(cpp, "ModelPartForComplex< %s::%s >::Hooks ",
				modulePath().c_str(), c->name().c_str());
		fprintf(cpp, "ModelPartForComplex< %s::%s >::hooks {\n",
				modulePath().c_str(), c->name().c_str());
		BOOST_FOREACH (const auto & dm, c->dataMembers()) {
			auto name = metaDataValue("slicer:name:", dm->getMetaData());
			fprintf(cpp, "\t\t{ \"%s\", ",
					name ? name->c_str() : dm->name().c_str());
			auto type = dm->type();
			fprintf(cpp, "new ModelPartForStruct< %s::%s >::Hook< %s, &%s::%s::%s, ",
				modulePath().c_str(), c->name().c_str(),
				Slice::typeToString(type).c_str(),
				modulePath().c_str(), c->name().c_str(), dm->name().c_str());
			createNewModelPartPtrFor(type);
			fprintf(cpp, " < %s > >() },\n",
				Slice::typeToString(type).c_str());
		}
		fprintf(cpp, "\t};\n\n");
		
		return true;
	}

	void
	Slicer::visitSequence(const Slice::SequencePtr & s)
	{
		if (s->hasMetaData("slicer:ignore")) { return; }

		fprintf(cpp, "// Sequence %s\n", s->name().c_str());
		fprintf(cpp, "template<>\n");
		fprintf(cpp, "ModelPartPtr ModelPartForSequence< %s::%s >::GetChild(const std::string & name)\n{\n",
				modulePath().c_str(), s->name().c_str());
		auto iname = metaDataValue("slicer:item:", s->getMetaData());
		if (iname) {
			fprintf(cpp, "\tif (!name.empty() && name != \"%s\") { throw IncorrectElementName(); }\n",
					iname->c_str());
		}
		else {
			fprintf(cpp, "\t(void)name;\n");
		}
		fprintf(cpp, "\tsequence.push_back(%s());\n",
					Slice::typeToString(s->type()).c_str());
		fprintf(cpp, "\treturn new ");
		auto etype = s->type();
		createNewModelPartPtrFor(etype);
		fprintf(cpp, "< %s >(sequence.back());\n}\n\n",
				Slice::typeToString(etype).c_str());
		fprintf(cpp, "template<>\n");
		fprintf(cpp, "ModelPartPtr\n");
		fprintf(cpp, "ModelPartForSequence< %s::%s >::elementModelPart(typename %s::%s::value_type & e) const {\n",
				modulePath().c_str(), s->name().c_str(),
				modulePath().c_str(), s->name().c_str());
		fprintf(cpp, "\treturn new ");
		createNewModelPartPtrFor(etype);
		fprintf(cpp, "< %s >(e);\n}\n\n",
				Slice::typeToString(etype).c_str());
		fprintf(cpp, "template<>\n");
		auto ename = metaDataValue("slicer:element:", s->getMetaData());
		fprintf(cpp, "std::string ModelPartForSequence< %s::%s >::elementName(\"%s\");\n\n",
				modulePath().c_str(), s->name().c_str(),
				ename ? ename->c_str() : "element");
	}

	void
	Slicer::visitDictionary(const Slice::DictionaryPtr & d)
	{
		if (d->hasMetaData("slicer:ignore")) { return; }

		fprintf(cpp, "// Dictionary %s\n", d->name().c_str());
		auto iname = metaDataValue("slicer:item:", d->getMetaData());
		fprintf(cpp, "template<>\n");
		fprintf(cpp, "std::string ModelPartForDictionary< %s::%s >::pairName(\"%s\");\n\n",
				modulePath().c_str(), d->name().c_str(),
				iname ? iname->c_str() : "element");

		fprintf(cpp, "template<>\n");
		fprintf(cpp, "ModelPartForComplex< ModelPartForDictionaryElement< %s::%s > >::Hooks ",
				modulePath().c_str(), d->name().c_str());
		fprintf(cpp, "ModelPartForComplex< ModelPartForDictionaryElement< %s::%s > >::hooks {\n",
				modulePath().c_str(), d->name().c_str());
		auto kname = metaDataValue("slicer:key:", d->getMetaData());
		auto vname = metaDataValue("slicer:value:", d->getMetaData());
		fprintf(cpp, "\t\t{ \"%s\", ",
				kname ? kname->c_str() : "key");
		auto ktype = d->keyType();
		fprintf(cpp, "new ModelPartForDictionaryElement< %s::%s >::Hook< %s*, &ModelPartForDictionaryElement< %s::%s >::key, ",
				modulePath().c_str(), d->name().c_str(),
				Slice::typeToString(ktype).c_str(),
				modulePath().c_str(), d->name().c_str());
		createNewModelPartPtrFor(ktype);
		fprintf(cpp, "< %s > >() },\n\t\t{ \"%s\", ",
				Slice::typeToString(ktype).c_str(),
				vname ? vname->c_str() : "value");
		auto vtype = d->valueType();
		fprintf(cpp, "new ModelPartForDictionaryElement< %s::%s >::Hook< %s*, &ModelPartForDictionaryElement< %s::%s >::value, ",
				modulePath().c_str(), d->name().c_str(),
				Slice::typeToString(vtype).c_str(),
				modulePath().c_str(), d->name().c_str());
		createNewModelPartPtrFor(vtype);
		fprintf(cpp, "< %s > >() },\n",
				Slice::typeToString(vtype).c_str());
		fprintf(cpp, "\t};\n");
		fprintf(cpp, "\n");
	}

	void
	Slicer::visitModuleEnd(const Slice::ModulePtr & m)
	{
		fprintf(cpp, "// End module %s\n\n", m->name().c_str());
		modules.pop_back();
	}

	void
	Slicer::createNewModelPartPtrFor(const Slice::TypePtr & type) const
	{
		if (auto builtin = Slice::BuiltinPtr::dynamicCast(type)) {
			fprintf(cpp, "ModelPartForSimple ");
		}
		else if (auto complexClass = Slice::ClassDeclPtr::dynamicCast(type)) {
			fprintf(cpp, "ModelPartForClass ");
		}
		else if (auto complexStruct = Slice::StructPtr::dynamicCast(type)) {
			fprintf(cpp, "ModelPartForStruct ");
		}
		else if (auto sequence = Slice::SequencePtr::dynamicCast(type)) {
			fprintf(cpp, "ModelPartForSequence ");
		}
		else if (auto dictionary = Slice::DictionaryPtr::dynamicCast(type)) {
			fprintf(cpp, "ModelPartForDictionary ");
		}
	}

	std::string
	Slicer::modulePath()
	{
		std::string path;
		BOOST_FOREACH (const auto & m, modules) {
			path += "::";
			path += m->name();
		}
		return path;
	}

	boost::optional<std::string>
	Slicer::metaDataValue(const std::string & prefix, const std::list<std::string> & metadata)
	{
		BOOST_FOREACH (const auto & md, metadata) {
			if (boost::algorithm::starts_with(md, prefix)) {
				return md.substr(prefix.length());
			}
		}
		return boost::optional<std::string>();
	}

	void
	Slicer::Apply(const boost::filesystem::path & ice, const boost::filesystem::path & cpp)
	{
		std::vector<std::string> cppArgs;
		Slice::PreprocessorPtr icecpp = Slice::Preprocessor::create("slicer", ice.string(), cppArgs);
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

		FilePtr cppfile(fopen(cpp.string().c_str(), "a"), fclose);
		if (!cppfile) {
			throw std::runtime_error("failed to open code file");
		}

		Slicer s(cppfile.get());
		u->visit(&s, false);

		u->destroy();
	}
};
