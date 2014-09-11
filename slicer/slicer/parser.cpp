#include "parser.h"
#include <Slice/Parser.h>
#include <Slice/Preprocessor.h>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <Slice/CPlusPlusUtil.h>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/convenience.hpp>

namespace fs = boost::filesystem;

namespace Slicer {
	Slicer::Slicer(FILE * c) :
		cpp(c),
		classNo(0)
	{
	}

	void
	Slicer::defineConversions(Slice::DataMemberPtr dm) const
	{
		auto type = dm->type();
		auto c = Slice::ContainedPtr::dynamicCast(dm->container());
		auto conversions = getConversions(dm);
		BOOST_FOREACH(const auto & conversion, conversions) {
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
			fprintf(cpp, "ModelPartForConverted< %s, %s::%s, &%s::%s::%s >::SetValue(ValueSourcePtr vsp)\n",
					Slice::typeToString(type).c_str(),
					modulePath().c_str(), c->name().c_str(),
					modulePath().c_str(), c->name().c_str(), dm->name().c_str());
			fprintf(cpp, "{\n");

			BOOST_FOREACH(const auto & conversion, conversions) {
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
			fprintf(cpp, "ModelPartForConverted< %s, %s::%s, &%s::%s::%s >::GetValue(ValueTargetPtr vtp)\n",
					Slice::typeToString(type).c_str(),
					modulePath().c_str(), c->name().c_str(),
					modulePath().c_str(), c->name().c_str(), dm->name().c_str());
			fprintf(cpp, "{\n");

			BOOST_FOREACH(const auto & conversion, conversions) {
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
		BOOST_FOREACH(const auto & c, m->structs()) {
			BOOST_FOREACH(const auto & dm, c->dataMembers()) {
				defineConversions(dm);
			}
		}
		BOOST_FOREACH(const auto & c, m->classes()) {
			BOOST_FOREACH(const auto & dm, c->dataMembers()) {
				defineConversions(dm);
			}
		}
		return true;
	}

	bool
	Slicer::visitClassDefStart(const Slice::ClassDefPtr & c)
	{
		if (c->isInterface()) { return false; }
		if (c->hasMetaData("slicer:ignore")) { return false; }

		auto decl = c->declaration();
		fprintf(cpp, "// Class %s\n", c->name().c_str());
		fprintf(cpp, "template<>\n");
		fprintf(cpp, "ModelPartForComplex< %s::%s >::Hooks ",
				modulePath().c_str(), c->name().c_str());
		fprintf(cpp, "ModelPartForComplex< %s::%s >::hooks {\n",
				modulePath().c_str(), c->name().c_str());
		visitComplexDataMembers(decl, c->allDataMembers());
		fprintf(cpp, "\t};\n\n");
		
		fprintf(cpp, "template<>\n");
		auto name = metaDataValue("slicer:root:", c->getMetaData());
		fprintf(cpp, "std::string ModelPartForClassRoot< %s >::rootName(\"%s\");\n\n",
				typeToString(decl).c_str(),
				name ? name->c_str() : c->name().c_str());

		fprintf(cpp, "static void registerClass_%u() __attribute__ ((constructor(210)));\n", classNo);
		fprintf(cpp, "static void registerClass_%u()\n{\n", classNo);
		fprintf(cpp, "\tSlicer::classRefMap()->insert({ \"%s::%s\", [](void * p){ return new ModelPartForClass< %s >(*static_cast< %s *>(p)); } });\n",
				modulePath().c_str(), c->name().c_str(),
				typeToString(decl).c_str(),
				typeToString(decl).c_str());
		fprintf(cpp, "}\n\n");
		fprintf(cpp, "static void unregisterClass_%u() __attribute__ ((destructor(210)));\n", classNo);
		fprintf(cpp, "static void unregisterClass_%u()\n{\n", classNo);
		fprintf(cpp, "\tSlicer::classRefMap()->erase(\"%s::%s\");\n",
				modulePath().c_str(), c->name().c_str());
		fprintf(cpp, "}\n\n");

		fprintf(cpp, "template<>\nTypeId\nModelPartForClass< %s >::GetTypeId() const\n{\n",
				typeToString(decl).c_str());
		fprintf(cpp, "\tauto id = ModelObject->ice_id();\n");
		fprintf(cpp, "\treturn (id == \"%s::%s\") ? TypeId() : id;\n}\n\n",
				modulePath().c_str(), c->name().c_str());

		classNo += 1;

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
		visitComplexDataMembers(c, c->dataMembers());
		fprintf(cpp, "\t};\n\n");
		
		return true;
	}

	void
	Slicer::visitComplexDataMembers(Slice::TypePtr it, const Slice::DataMemberList & dataMembers) const
	{
		BOOST_FOREACH (const auto & dm, dataMembers) {
			auto c = Slice::ContainedPtr::dynamicCast(dm->container());
			auto t = Slice::TypePtr::dynamicCast(dm->container());
			if (!t) {
				t = Slice::ClassDefPtr::dynamicCast(dm->container())->declaration();
			}
			auto name = metaDataValue("slicer:name:", dm->getMetaData());
			auto conversions = metaDataValues("slicer:conversion:", dm->getMetaData());
			fprintf(cpp, "\t\tnew ");
			auto type = dm->type();
			createNewModelPartPtrFor(t);
			fprintf(cpp, "< %s >::Hook< %s",
					typeToString(it).c_str(),
					Slice::typeToString(type, dm->optional()).c_str());
			fprintf(cpp, ", %s::%s, &%s::%s::%s, ",
					modulePath().c_str(), c->name().c_str(),
					modulePath().c_str(), c->name().c_str(), dm->name().c_str());
			if (dm->optional()) {
				fprintf(cpp, "ModelPartForOptional< ");
			}
			if (!conversions.empty()) {
				fprintf(cpp, "ModelPartForConverted< %s, %s::%s, &%s::%s::%s >",
						Slice::typeToString(type).c_str(),
						modulePath().c_str(), c->name().c_str(),
						modulePath().c_str(), c->name().c_str(), dm->name().c_str());
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
		fprintf(cpp, "\tsequence.push_back(typename element_type::value_type());\n");
		fprintf(cpp, "\treturn new ");
		auto etype = s->type();
		createNewModelPartPtrFor(etype);
		fprintf(cpp, "<typename element_type::value_type>(sequence.back());\n}\n\n");
		fprintf(cpp, "template<>\n");
		fprintf(cpp, "ModelPartPtr\n");
		fprintf(cpp, "ModelPartForSequence< %s::%s >::elementModelPart(typename %s::%s::value_type & e) const {\n",
				modulePath().c_str(), s->name().c_str(),
				modulePath().c_str(), s->name().c_str());
		fprintf(cpp, "\treturn new ");
		createNewModelPartPtrFor(etype);
		fprintf(cpp, "<typename element_type::value_type>(e);\n}\n\n");
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
		fprintf(cpp, "\t\t");
		auto ktype = d->keyType();
		fprintf(cpp, "new ModelPartForDictionaryElement< %s::%s >::Hook< %s*, ModelPartForDictionaryElement< %s::%s >, &ModelPartForDictionaryElement< %s::%s >::key, ",
				modulePath().c_str(), d->name().c_str(),
				Slice::typeToString(ktype).c_str(),
				modulePath().c_str(), d->name().c_str(),
				modulePath().c_str(), d->name().c_str());
		createNewModelPartPtrFor(ktype);
		fprintf(cpp, "< %s > >(\"%s\"),\n\t\t",
				Slice::typeToString(ktype).c_str(),
				kname ? kname->c_str() : "key");
		auto vtype = d->valueType();
		fprintf(cpp, "new ModelPartForDictionaryElement< %s::%s >::Hook< %s*, ModelPartForDictionaryElement< %s::%s >, &ModelPartForDictionaryElement< %s::%s >::value, ",
				modulePath().c_str(), d->name().c_str(),
				Slice::typeToString(vtype).c_str(),
				modulePath().c_str(), d->name().c_str(),
				modulePath().c_str(), d->name().c_str());
		createNewModelPartPtrFor(vtype);
		fprintf(cpp, "< %s > >(\"%s\"),\n",
				Slice::typeToString(vtype).c_str(),
				vname ? vname->c_str() : "value");
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
	}

	std::string
	Slicer::modulePath() const
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

	std::list<std::string>
	Slicer::metaDataValues(const std::string & prefix, const std::list<std::string> & metadata)
	{
		std::list<std::string> mds;
		BOOST_FOREACH (const auto & md, metadata) {
			if (boost::algorithm::starts_with(md, prefix)) {
				mds.push_back(md.substr(prefix.length()));
			}
		}
		return mds;
	}

	std::vector<std::string>
	Slicer::metaDataSplit(const std::string & metadata)
	{
		std::vector<std::string> parts;
		boost::algorithm::split(parts, metadata, boost::algorithm::is_any_of(":"), boost::algorithm::token_compress_off); 
		return parts;	
	}

	std::vector<Slicer::ConversionSpec>
	Slicer::getConversions(Slice::DataMemberPtr dm)
	{
		std::vector<ConversionSpec> rtn;
		auto conversions = metaDataValues("slicer:conversion:", dm->getMetaData());
		BOOST_FOREACH(const auto & conversion, conversions) {
			auto split = metaDataSplit(conversion);
			if (split.size() != 3) {
				throw std::runtime_error("conversion needs 3 parts type:toModelFunc:toExchangeFunc");
			}
			BOOST_FOREACH(auto & p, split) {
				boost::algorithm::replace_all(p, ".", "::");
			}
			rtn.push_back(ConversionSpec({split[0], split[1], split[2]}));
		}
		return rtn;
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

