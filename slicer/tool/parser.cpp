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
			createModelPartForConverted(type, c->scoped(), dm);
			fprintbf(cpp, "::SetValue(ValueSourcePtr vsp)\n{\n");
			fprintbf(cpp, "\tBOOST_ASSERT(Model);\n");

			for (const auto & conversion : conversions) {
				fprintbf(cpp, "\tif (tryConvertFrom< %s >(vsp, Model, &%s)) return;\n",
						conversion.ExchangeType,
						conversion.ConvertToModelFunc);
			}
			// Default conversion
			if (!dm->hasMetaData("slicer:nodefaultconversion")) {
				fprintbf(cpp, "\tif (tryConvertFrom< %s >(vsp, Model)) return;\n",
						Slice::typeToString(type));
			}
			// Failed to convert
			fprintbf(cpp, "\tthrow NoConversionFound(\"%s\");\n",
					Slice::typeToString(type));
			fprintbf(cpp, "}\n\n");

			fprintbf(cpp, "template<> DLL_PUBLIC\nvoid\n");
			createModelPartForConverted(type, c->scoped(), dm);
			fprintbf(cpp, "::GetValue(ValueTargetPtr vtp)\n{\n");
			fprintbf(cpp, "\tBOOST_ASSERT(Model);\n");

			for (const auto & conversion : conversions) {
				fprintbf(cpp, "\tif (tryConvertTo< %s >(vtp, Model, &%s)) return;\n",
						conversion.ExchangeType,
						conversion.ConvertToExchangeFunc);
			}
			// Default conversion
			if (!dm->hasMetaData("slicer:nodefaultconversion")) {
				fprintbf(cpp, "\tif (tryConvertTo< %s >(vtp, Model)) return;\n",
					Slice::typeToString(type));
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
	Slicer::defineRoot(const std::string & type, const std::string & name, const Slice::TypePtr & stype) const
	{
		if (stype->isLocal()) {
			fprintbf(cpp, "template<>\n");
			fprintbf(cpp, "struct isLocal< %s > { static constexpr bool value = true; };\n\n",
					type);
		}
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
				decl->typeId(),
				typeId ? *typeId : "slicer-typeid");

		auto name = metaDataValue("slicer:root:", c->getMetaData());
		defineRoot(typeToString(decl), name ? *name : c->name(), decl);

		auto typeName = metaDataValue("slicer:typename:", c->getMetaData());
		fprintbf(cpp, "template<> DLL_PUBLIC\n");
		fprintbf(cpp, "const std::string * ModelPartForClass< %s >::className = nullptr;\n",
				decl->typeId());
		fprintbf(cpp, "template<> DLL_PUBLIC\n");
		fprintbf(cpp, "const std::string * ModelPartForClass< %s >::typeName = nullptr;\n",
				decl->typeId());
		fprintbf(cpp, "template<>\nvoid ModelPartForClass< %s >::initClassName() {\n\tclassName = new std::string(\"%s\");\n\t",
				decl->typeId(), c->scoped());
		if (typeName) {
			fprintbf(cpp, "typeName = new std::string(\"%s\");",
					*typeName);
		}
		else {
			fprintbf(cpp, "typeName = nullptr;");
		}
		fprintbf(cpp, "\n}\n");

		fprintbf(cpp, "template<> DLL_PUBLIC\nconst Metadata ModelPartForComplex< %s >::metadata ",
				c->scoped());
		copyMetadata(c->getMetaData());
		fprintbf(cpp, ";\n\n");

		if (auto cmp = metaDataValue("slicer:custommodelpart:", c->getMetaData())) {
			fprintbf(cpp, "CUSTOMMODELPARTFOR(%s, %s< %s >, %s);\n\n",
					Slice::typeToString(decl), getBasicModelPart(decl), c->scoped(), boost::algorithm::replace_all_copy(*cmp, ".", "::"));
		}
		else {
			fprintbf(cpp, "CUSTOMMODELPARTFOR(%s, ModelPartForClass<%s>, ModelPartForClass<%s>);\n\n",
					Slice::typeToString(decl), c->scoped(), c->scoped());
		}

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
		defineRoot(c->scoped(), name ? *name : c->name(), c);

		fprintbf(cpp, "template<> DLL_PUBLIC\nconst Metadata ModelPartForComplex< %s >::metadata ",
				c->scoped());
		copyMetadata(c->getMetaData());
		fprintbf(cpp, ";\n\n");

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
			if (hasMetadata(dm->getMetaData())) {
				fprintbf(cpp, "::HookMetadata<");
			}
			else {
				fprintbf(cpp, "::Hook<");
			}
			fprintbf(cpp, " %s, ",
					Slice::typeToString(type, dm->optional()));
			createNewModelPartPtrFor(type, dm, getAllMetadata(dm));
			fprintbf(cpp, " >(&%s, \"%s\"",
					dm->scoped(),
					name ? *name : dm->name());
			if (hasMetadata(dm->getMetaData())) {
				fprintbf(cpp, ", ");
				copyMetadata(dm->getMetaData());
			}
			fprintbf(cpp, " ),\n");
		}
		fprintbf(cpp, "\t};\n\n");
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
		fprintbf(cpp, ";\n\n");

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

		auto name = metaDataValue("slicer:root:", e->getMetaData());
		defineRoot(e->scoped(), name ? *name : e->name(), e);

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
		auto ename = metaDataValue("slicer:element:", s->getMetaData());
		fprintbf(cpp, "const std::string ModelPartForSequence< %s >::elementName(\"%s\");\n\n",
				s->scoped(),
				ename ? *ename : "element");

		auto name = metaDataValue("slicer:root:", s->getMetaData());
		defineRoot(s->scoped(), name ? *name : s->name(), s);

		fprintbf(cpp, "template<> DLL_PUBLIC\nconst Metadata ModelPartForSequence< %s >::metadata ",
				s->scoped());
		copyMetadata(s->getMetaData());
		fprintbf(cpp, ";\n\n");

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
		fprintbf(cpp, "new ModelPartForComplex< %s::value_type >::Hook< const %s, ",
				d->scoped(),
				Slice::typeToString(ktype));
		createNewModelPartPtrFor(ktype);
		fprintbf(cpp, " >(&%s::value_type::first, \"%s\"),\n\t\t",
				d->scoped(),
				kname ? *kname : "key");
		auto vtype = d->valueType();
		fprintbf(cpp, "new ModelPartForComplex< %s::value_type >::Hook< %s, ",
				d->scoped(),
				Slice::typeToString(vtype));
		createNewModelPartPtrFor(vtype);
		fprintbf(cpp, " >(&%s::value_type::second, \"%s\"),\n",
				d->scoped(),
				vname ? *vname : "value");
		fprintbf(cpp, "\t};\n");
		fprintbf(cpp, "\n");

		auto name = metaDataValue("slicer:root:", d->getMetaData());
		defineRoot(d->scoped(), name ? *name : d->name(), d);

		fprintbf(cpp, "template<> DLL_PUBLIC\nconst Metadata ModelPartForDictionary< %s >::metadata ",
				d->scoped());
		copyMetadata(d->getMetaData());
		fprintbf(cpp, ";\n\n");

		fprintbf(cpp, "template<> DLL_PUBLIC\nconst Metadata ModelPartForComplex<%s::value_type>::metadata ",
				d->scoped());
		copyMetadata(d->getMetaData());
		fprintbf(cpp, ";\n\n");

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
	Slicer::createModelPartForConverted(const Slice::TypePtr & type, const std::string & container, const Slice::DataMemberPtr & dm) const
	{
		fprintbf(cpp, "ModelPartForConverted< ");
		if (dm->optional()) {
			fprintbf(cpp, "IceUtil::Optional< %s >",
					Slice::typeToString(type));
		}
		else {
			fprintbf(cpp, "%s",
					Slice::typeToString(type));
		}
		fprintbf(cpp, ", %s, &%s >",
				container,
				dm->scoped());
	}

	void
	Slicer::createNewModelPartPtrFor(const Slice::TypePtr & type, const Slice::DataMemberPtr & dm, const Slice::StringList & md) const
	{
		auto conversions = getConversions(md);
		if (dm && !conversions.empty()) {
			createModelPartForConverted(type,
					boost::algorithm::trim_right_copy_if(dm->container()->thisScope(), ispunct),
					dm);
		}
		else if (auto cmp = metaDataValue("slicer:custommodelpart:", md)) {
			fprintbf(cpp, "%s",
				boost::algorithm::replace_all_copy(*cmp, ".", "::"));
		}
		else {
			if (dm && dm->optional()) {
				fprintbf(cpp, "ModelPartForOptional< ");
			}
			fprintbf(cpp, "%s< %s >",
					getBasicModelPart(type), Slice::ClassDeclPtr::dynamicCast(type) ? type->typeId() : Slice::typeToString(type));
			if (dm && dm->optional()) {
				fprintbf(cpp, " > ");
			}
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
		fprintbf(cpp, "}");
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

