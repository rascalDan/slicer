#include "parser.h"
#include <metadata.h>
#include <common.h>
#include <Slice/Parser.h>
#include <Slice/Preprocessor.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <Slice/CPlusPlusUtil.h>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/convenience.hpp>
#include <fprintbf.h>
#include <safeMapFind.h>

namespace fs = boost::filesystem;

namespace Slicer {
	Slicer::Slicer() :
		cpp(NULL),
		headerPrefix("slicer"),
		allowIcePrefix(false),
		components(0),
		classNo(0)
	{
	}

	void
	Slicer::defineConversions(Slice::DataMemberPtr dm) const
	{
		if (!cpp) return;

		auto type = dm->type();
		auto c = Slice::ContainedPtr::dynamicCast(dm->container());
		auto conversions = getConversions(getAllMetadata(dm));
		for (const auto & conversion : conversions) {
			if (!AdHoc::containerContains(conversion.Options, "nodeclare")) {
				if (!AdHoc::containerContains(conversion.Options, "nodeclareto")) {
					fprintbf(cpp, "DLL_PUBLIC %s %s(const %s &);\n",
							conversion.ExchangeType,
							conversion.ConvertToExchangeFunc,
							Slice::typeToString(type));
				}
				if (!AdHoc::containerContains(conversion.Options, "nodeclarefrom")) {
					fprintbf(cpp, "DLL_PUBLIC %s %s(const %s &);\n\n",
							Slice::typeToString(type),
							conversion.ConvertToModelFunc,
							conversion.ExchangeType);
				}
			}
		}
		if (!conversions.empty()) {
			fprintbf(cpp, "template<> DLL_PUBLIC\nvoid\n");
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
				fprintbf(cpp, "\t\tModel = %s(tmp);\n",
						conversion.ConvertToModelFunc);
				fprintbf(cpp, "\t\treturn;\n");
				fprintbf(cpp, "\t}\n");
			}
			// Default conversion
			if (!dm->hasMetaData("slicer:nodefaultconversion")) {
				fprintbf(cpp, "\tif (auto vspt = dynamic_cast<TValueSource< %s > *>(vsp.get())) {\n",
						Slice::typeToString(type));
				fprintbf(cpp, "\t\tvspt->set(Model);\n");
				fprintbf(cpp, "\t\treturn;\n");
				fprintbf(cpp, "\t}\n");
			}
			// Failed to convert
			fprintbf(cpp, "\tthrow NoConversionFound(\"%s\");\n",
					Slice::typeToString(type));
			fprintbf(cpp, "}\n\n");

			fprintbf(cpp, "template<> DLL_PUBLIC\nvoid\n");
			fprintbf(cpp, "ModelPartForConverted< %s, %s, &%s >::GetValue(ValueTargetPtr vtp)\n",
					Slice::typeToString(type),
					c->scoped(),
					dm->scoped());
			fprintbf(cpp, "{\n");

			for (const auto & conversion : conversions) {
				fprintbf(cpp, "\tif (auto vtpt = dynamic_cast<TValueTarget< %s > *>(vtp.get())) {\n",
						conversion.ExchangeType);
				fprintbf(cpp, "\t\tvtpt->get(%s(Model));\n",
						conversion.ConvertToExchangeFunc);
				fprintbf(cpp, "\t\treturn;\n");
				fprintbf(cpp, "\t}\n");
			}
			// Default conversion
			if (!dm->hasMetaData("slicer:nodefaultconversion")) {
				fprintbf(cpp, "\tif (auto vtpt = dynamic_cast<TValueTarget< %s > *>(vtp.get())) {\n",
					Slice::typeToString(type));
				fprintbf(cpp, "\t\tvtpt->get(Model);\n");
				fprintbf(cpp, "\t\treturn;\n");
				fprintbf(cpp, "\t}\n");
			}
			// Failed to convert
			fprintbf(cpp, "\tthrow NoConversionFound(\"%s\");\n",
					Slice::typeToString(type));
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
		fprintbf(cpp, "#include <%s>\n", (headerPrefix / "modelPartsTypes.impl.h").string());
		fprintbf(cpp, "#include <%s>\n", (headerPrefix / "common.h").string());
		for (const auto & m : u->modules()) {
			for (const auto & i : metaDataValues("slicer:include:", m->getMetaData())) {
				fprintbf(cpp, "#include <%s>\n", i);
			}
		}
		fprintbf(cpp, "\n");
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
		fprintbf(cpp, "template<> DLL_PUBLIC\n");
		fprintbf(cpp, "const std::string ModelPartForRoot< %s >::rootName(\"%s\");\n\n",
				type, name);
		fprintbf(cpp, "template<> DLL_PUBLIC\n");
		fprintbf(cpp, "const std::string ModelPartForRoot< IceUtil::Optional< %s > >::rootName(\"Optional%s\");\n\n",
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

		fprintbf(cpp, "template<> DLL_PUBLIC\n");
		auto typeId = metaDataValue("slicer:typeid:", c->getMetaData());
		fprintbf(cpp, "const std::string ModelPartForClass< %s >::typeIdProperty(\"%s\");\n\n",
				typeToString(decl),
				typeId ? *typeId : "slicer-typeid");

		auto name = metaDataValue("slicer:root:", c->getMetaData());
		defineRootName(typeToString(decl), name ? *name : c->name());

		auto typeName = metaDataValue("slicer:typename:", c->getMetaData());
		fprintbf(cpp, "template<> DLL_PUBLIC\n");
		fprintbf(cpp, "__attribute__ ((init_priority(209)))\nconst std::string ModelPartForClass< %s >::className(\"%s\");\n",
				typeToString(decl), c->scoped());
		fprintbf(cpp, "template<> DLL_PUBLIC\n");
		fprintbf(cpp, "__attribute__ ((init_priority(209)))\nconst IceUtil::Optional<std::string> ModelPartForClass< %s >::typeName",
				typeToString(decl));
		if (typeName) {
			fprintbf(cpp, "(\"%s\")",
					*typeName);
		}
		else {
			fprintbf(cpp, "(IceUtil::None)");
		}
		fprintbf(cpp, ";\n\n");

		fprintbf(cpp, "template<> DLL_PUBLIC\nconst Metadata ModelPartForComplex< %s >::metadata ",
				c->scoped());
		copyMetadata(c->getMetaData());

		defineMODELPART(stringbf("::IceInternal::Handle< %s >", c->scoped()), decl, c->getMetaData());

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

		fprintbf(cpp, "template<> DLL_PUBLIC\nconst Metadata ModelPartForComplex< %s >::metadata ",
				c->scoped());
		copyMetadata(c->getMetaData());

		defineMODELPART(c->scoped(), c, c->getMetaData());

		return true;
	}

	void
	Slicer::visitComplexDataMembers(Slice::ConstructedPtr it, const Slice::DataMemberList & dataMembers) const
	{
		if (!cpp) return;

		fprintbf(cpp, "template<> DLL_PUBLIC\n");
		fprintbf(cpp, "const ModelPartForComplex< %s >::Hooks ",
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
			fprintbf(cpp, "\t\tnew ");
			auto type = dm->type();
			createNewModelPartPtrFor(it);
			fprintbf(cpp, "::Hook< %s",
					Slice::typeToString(type, dm->optional()));
			fprintbf(cpp, ", %s, &%s, ",
					boost::algorithm::trim_right_copy_if(dm->container()->thisScope(), ispunct),
					dm->scoped());
			if (dm->optional()) {
				fprintbf(cpp, "ModelPartForOptional< ");
			}
			createNewModelPartPtrFor(type, dm, getAllMetadata(dm));
			if (dm->optional()) {
				fprintbf(cpp, " > ");
			}
			if (!hasMetadata(dm->getMetaData())) {
				fprintbf(cpp, ", HookBase");
			}
			fprintbf(cpp, " >(\"%s\"),\n",
					name ? *name : dm->name());
		}
		fprintbf(cpp, "\t};\n\n");

		for (const auto & dm : dataMembers) {
			if (!hasMetadata(dm->getMetaData())) {
				continue;
			}
			auto c = Slice::ContainedPtr::dynamicCast(dm->container());
			auto t = Slice::TypePtr::dynamicCast(dm->container());
			if (!t) {
				t = Slice::ClassDefPtr::dynamicCast(dm->container())->declaration();
			}
			auto type = dm->type();
			fprintbf(cpp, "template<>\ntemplate<> DLL_PUBLIC\nconst Metadata\n");
			createNewModelPartPtrFor(it);
			fprintbf(cpp, "::HookMetadata< %s",
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
		fprintbf(cpp, "template<> DLL_PUBLIC\nconst Metadata ModelPartForEnum< %s >::metadata ",
				e->scoped());
		copyMetadata(e->getMetaData());

		fprintbf(cpp, "template<> DLL_PUBLIC\nconst ModelPartForEnum< %s >::Enumerations\nModelPartForEnum< %s >::enumerations([]() -> ModelPartForEnum< %s >::Enumerations\n",
				e->scoped(),
				e->scoped(),
				e->scoped());
		fprintbf(cpp, "{\n\tModelPartForEnum< %s >::Enumerations e;\n",
				e->scoped());
		for (const auto & ee : e->getEnumerators()) {
			fprintbf(cpp, "\te.insert( { %s, \"%s\" } );\n", ee->scoped(), ee->name());
		}
		fprintbf(cpp, "\treturn e;\n}());\n\n");

		fprintbf(cpp, "template<> DLL_PUBLIC\nvoid ModelPartForEnum< %s >::SetValue(ValueSourcePtr s) {\n\
	std::string val;\n\
	s->set(val);\n\
	this->Model = lookup(val);\n\
}\n\n",
				e->scoped());
		fprintbf(cpp, "template<> DLL_PUBLIC\nvoid ModelPartForEnum< %s >::GetValue(ValueTargetPtr s) {\n\
	s->get(lookup(this->Model));\n\
}\n\n",
				e->scoped());

		auto name = metaDataValue("slicer:root:", e->getMetaData());
		defineRootName(e->scoped(), name ? *name : e->name());

		defineMODELPART(e->scoped(), e, e->getMetaData());
	}

	void
	Slicer::visitSequence(const Slice::SequencePtr & s)
	{
		if (s->hasMetaData("slicer:ignore")) { return; }

		components += 1;

		if (!cpp) return;

		fprintbf(cpp, "// Sequence %s\n", s->name());
		fprintbf(cpp, "template<> DLL_PUBLIC\n");
		fprintbf(cpp, "ChildRefPtr ModelPartForSequence< %s >::GetChildRef(const std::string & name, const HookFilter & flt)\n{\n",
				s->scoped());
		auto iname = metaDataValue("slicer:item:", s->getMetaData());
		if (iname) {
			fprintbf(cpp, "\tif (!name.empty() && name != \"%s\") { throw IncorrectElementName(name); }\n",
					*iname);
		}
		else {
			fprintbf(cpp, "\t(void)name;\n");
		}
		fprintbf(cpp, "\treturn GetAnonChildRef(flt);\n}\n\n");

		fprintbf(cpp, "template<> DLL_PUBLIC\n");
		fprintbf(cpp, "ModelPartPtr\n");
		fprintbf(cpp, "ModelPartForSequence< %s >::elementModelPart(typename %s::value_type & e) const {\n",
				s->scoped(),
				s->scoped());
		fprintbf(cpp, "\treturn ModelPart::CreateFor(e);\n}\n\n");

		fprintbf(cpp, "template<> DLL_PUBLIC\n");
		auto ename = metaDataValue("slicer:element:", s->getMetaData());
		fprintbf(cpp, "const std::string ModelPartForSequence< %s >::elementName(\"%s\");\n\n",
				s->scoped(),
				ename ? *ename : "element");

		auto name = metaDataValue("slicer:root:", s->getMetaData());
		defineRootName(s->scoped(), name ? *name : s->name());

		fprintbf(cpp, "template<> DLL_PUBLIC\nconst Metadata ModelPartForSequence< %s >::metadata ",
				s->scoped());
		copyMetadata(s->getMetaData());

		defineMODELPART(s->scoped(), s, s->getMetaData());
	}

	void
	Slicer::visitDictionary(const Slice::DictionaryPtr & d)
	{
		if (d->hasMetaData("slicer:ignore")) { return; }

		components += 1;

		if (!cpp) return;

		fprintbf(cpp, "// Dictionary %s\n", d->name());
		auto iname = metaDataValue("slicer:item:", d->getMetaData());
		fprintbf(cpp, "template<> DLL_PUBLIC\n");
		fprintbf(cpp, "const std::string ModelPartForDictionary< %s >::pairName(\"%s\");\n\n",
				d->scoped(),
				iname ? *iname : "element");

		fprintbf(cpp, "template<> DLL_PUBLIC\n");
		fprintbf(cpp, "const ModelPartForComplex< %s::value_type >::Hooks ",
				d->scoped());
		fprintbf(cpp, "ModelPartForComplex< %s::value_type >::hooks {\n",
				d->scoped());
		auto kname = metaDataValue("slicer:key:", d->getMetaData());
		auto vname = metaDataValue("slicer:value:", d->getMetaData());
		fprintbf(cpp, "\t\t");
		auto ktype = d->keyType();
		fprintbf(cpp, "new ModelPartForComplex< %s::value_type >::Hook< const %s, %s::value_type, &%s::value_type::first, ",
				d->scoped(),
				Slice::typeToString(ktype),
				d->scoped(),
				d->scoped());
		createNewModelPartPtrFor(ktype);
		fprintbf(cpp, ", HookBase >(\"%s\"),\n\t\t",
				kname ? *kname : "key");
		auto vtype = d->valueType();
		fprintbf(cpp, "new ModelPartForComplex< %s::value_type >::Hook< %s, %s::value_type, &%s::value_type::second, ",
				d->scoped(),
				Slice::typeToString(vtype),
				d->scoped(),
				d->scoped());
		createNewModelPartPtrFor(vtype);
		fprintbf(cpp, ", HookBase >(\"%s\"),\n",
				vname ? *vname : "value");
		fprintbf(cpp, "\t};\n");
		fprintbf(cpp, "\n");

		auto name = metaDataValue("slicer:root:", d->getMetaData());
		defineRootName(d->scoped(), name ? *name : d->name());

		fprintbf(cpp, "template<> DLL_PUBLIC\nconst Metadata ModelPartForDictionary< %s >::metadata ",
				d->scoped());
		copyMetadata(d->getMetaData());

		fprintbf(cpp, "template<> DLL_PUBLIC\nconst Metadata ModelPartForComplex<%s::value_type>::metadata ",
				d->scoped());
		copyMetadata(d->getMetaData());

		defineMODELPART(d->scoped(), d, d->getMetaData());
	}

	void
	Slicer::visitModuleEnd(const Slice::ModulePtr & m)
	{
		if (cpp) {
			fprintbf(cpp, "// End module %s\n\n", m->name());
		}
	}

	void
	Slicer::createNewModelPartPtrFor(const Slice::TypePtr & type, const Slice::DataMemberPtr & dm, const Slice::StringList & md) const
	{
		auto conversions = getConversions(md);
		if (dm && !conversions.empty()) {
			fprintbf(cpp, "ModelPartForConverted< %s, %s, &%s >",
					Slice::typeToString(type),
					boost::algorithm::trim_right_copy_if(dm->container()->thisScope(), ispunct),
					dm->scoped());
		}
		else if (auto cmp = metaDataValue("slicer:custommodelpart:", md)) {
			fprintbf(cpp, "%s",
				boost::algorithm::replace_all_copy(*cmp, ".", "::"));
		}
		else {
			fprintbf(cpp, "%s< %s >",
					getBasicModelPart(type), Slice::typeToString(type));
		}
	}

	std::string
	Slicer::getBasicModelPart(const Slice::TypePtr & type) const
	{
		if (auto builtin = Slice::BuiltinPtr::dynamicCast(type)) {
			return "ModelPartForSimple";
		}
		else if (auto complexClass = Slice::ClassDeclPtr::dynamicCast(type)) {
			return "ModelPartForClass";
		}
		else if (auto complexStruct = Slice::StructPtr::dynamicCast(type)) {
			return "ModelPartForStruct";
		}
		else if (auto sequence = Slice::SequencePtr::dynamicCast(type)) {
			return "ModelPartForSequence";
		}
		else if (auto dictionary = Slice::DictionaryPtr::dynamicCast(type)) {
			return "ModelPartForDictionary";
		}
		else if (auto enumeration = Slice::EnumPtr::dynamicCast(type)) {
			return "ModelPartForEnum";
		}
		throw CompilerError("Unknown basic type");
	}

	bool
	Slicer::hasMetadata(const std::list<std::string> & metadata) const
	{
		for (const auto & md : metadata) {
			if (boost::algorithm::starts_with(md, "slicer:")) {
				return true;
			}
		}
		return false;
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

	Slice::StringList
	Slicer::getAllMetadata(const Slice::DataMemberPtr & dm)
	{
		auto metadata = dm->getMetaData();
		auto typec = Slice::ContainedPtr::dynamicCast(dm->type());
		if (typec) {
			if (auto cd = Slice::ClassDeclPtr::dynamicCast(typec)) {
				typec = cd->definition();
			}
			if (typec) {
				auto typeMetadata = typec->getMetaData();
				std::copy(typeMetadata.begin(), typeMetadata.end(), std::back_inserter(metadata));
			}
		}
		return metadata;
	}

	Slicer::Conversions
	Slicer::getConversions(const std::list<std::string> & dm)
	{
		Conversions rtn;
		auto conversions = metaDataValues("slicer:conversion:", dm);
		for (const auto & conversion : conversions) {
			auto split = metaDataSplit(conversion);
			if (split.size() < 3) {
				throw CompilerError("conversion needs at least 3 parts type:toModelFunc:toExchangeFunc[:options]");
			}
			for (auto & pi : {0, 1, 2}) {
				boost::algorithm::replace_all(split[pi], ".", "::");
			}
			rtn.push_back(split);
		}
		return rtn;
	}

	void
	Slicer::defineMODELPART(const std::string & type, const Slice::TypePtr & stype, const Slice::StringList & metadata) const
	{
		if (auto cmp = metaDataValue("slicer:custommodelpart:", metadata)) {
			fprintbf(cpp, "CUSTOMMODELPARTFOR(%s, %s< %s >, %s);\n\n",
					type, getBasicModelPart(stype), type, boost::algorithm::replace_all_copy(*cmp, ".", "::"));
		}
		else {
			fprintbf(cpp, "MODELPARTFOR(%s, %s);\n\n",
					type, getBasicModelPart(stype));
		}
	}

	unsigned int
	Slicer::Components() const
	{
		return components;
	}

	unsigned int
	Slicer::Execute()
	{
		if (cpp != NULL && !cppPath.empty()) {
			throw CompilerError("Both file handle and path provided.");
		}
		FilePtr cppfile(
			cpp || cppPath.empty() ? cpp : fopen(cppPath.string(), "w"),
			cppPath.empty() ? fflush : fclose);
		if (!cppfile && !cppPath.empty()) {
			throw CompilerError("Failed to open output file");
		}
		try {
			cpp = cppfile.get();
			Slicer::Slicer::Args args;
			// Copy includes to args
			for(const auto & include : includes) {
				args.push_back("-I" + include.string());
			}

			Slice::PreprocessorPtr icecpp = Slice::Preprocessor::create("slicer", slicePath.string(), args);
			FILE * cppHandle = icecpp->preprocess(false);

			if (cppHandle == NULL) {
				throw CompilerError("preprocess failed");
			}

			Slice::UnitPtr u = Slice::Unit::createUnit(false, false, allowIcePrefix, false);

			int parseStatus = u->parse(slicePath.string(), cppHandle, false);

			if (!icecpp->close()) {
				throw CompilerError("preprocess close failed");
			}

			if (parseStatus == EXIT_FAILURE) {
				throw CompilerError("unit parse failed");
			}

			unsigned int initial = Components();

			u->visit(this, false);

			u->destroy();

			if (!cppPath.empty()) {
				cpp = NULL;
			}

			return Components() - initial;
		}
		catch (...) {
			if (!cppPath.empty()) {
				unlink(cppPath.c_str());
			}
			throw;
		}
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

