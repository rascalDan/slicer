#include "parser.h"
#include "icemetadata.h"
#include <IceUtil/Handle.h>
#include <Slice/CPlusPlusUtil.h>
#include <Slice/Parser.h>
#include <Slice/Preprocessor.h>
#include <algorithm>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/constants.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/format.hpp>
#include <cctype>
#include <common.h>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fprintbf.h>
#include <iterator>
#include <list>
#include <memory>
#include <optional>
#include <ostream>
#include <safeMapFind.h>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
// IWYU pragma: no_include <boost/algorithm/string/detail/classification.hpp>
// IWYU pragma: no_include <boost/function/function_base.hpp>
// IWYU pragma: no_include <boost/iterator/iterator_facade.hpp>
// IWYU pragma: no_include <boost/iterator/iterator_traits.hpp>
// IWYU pragma: no_include <boost/range/begin.hpp>
// IWYU pragma: no_include <boost/range/end.hpp>
// IWYU pragma: no_include <boost/type_index/type_index_facade.hpp>

namespace fs = std::filesystem;

namespace Slicer {
	template<typename TPtr>
	bool
	ignoreType(const TPtr & t)
	{
		return (!t || t->hasMetaData("slicer:ignore"));
	}

	class Count : public Slice::ParserVisitor {
	public:
		[[nodiscard]] bool
		visitClassDefStart(const Slice::ClassDefPtr & c) override
		{
			return countIfUsed(c, classes, !c->isInterface());
		}

		[[nodiscard]] bool
		visitStructStart(const Slice::StructPtr & s) override
		{
			return countIfUsed(s, structs);
		}

		void
		visitSequence(const Slice::SequencePtr & s) override
		{
			countIfUsed(s, sequences);
		}

		void
		visitDictionary(const Slice::DictionaryPtr & d) override
		{
			countIfUsed(d, dictionaries);
		}

		void
		visitEnum(const Slice::EnumPtr & e) override
		{
			countIfUsed(e, enums);
		}

		[[nodiscard]] auto
		complexes() const
		{
			return classes + structs;
		}

		unsigned int classes {0};
		unsigned int structs {0};
		unsigned int enums {0};
		unsigned int sequences {0};
		unsigned int dictionaries {0};
		unsigned int total {0};

	private:
		template<typename TPtr>
		bool
		countIfUsed(const TPtr & t, auto & counter, bool condition = true)
		{
			if (!condition || ignoreType(t)) {
				return false;
			}
			counter += 1;
			total += 1;
			return true;
		}
	};

	class ForwardDeclare : public Slice::ParserVisitor {
	public:
		ForwardDeclare(FILE * c, const Count & cnt) : cpp(c), count(cnt) { }

		bool
		visitModuleStart(const Slice::ModulePtr & m) override
		{
			if (count.classes || count.structs || count.enums) {
				fprintbf(cpp, "// NOLINTNEXTLINE(modernize-concat-nested-namespaces)\n");
				fprintbf(cpp, "namespace %s {\n", m->name());
				return true;
			}
			return false;
		}

		bool
		visitClassDefStart(const Slice::ClassDefPtr & c) override
		{
			fprintbf(cpp, "class ICE_CLASS(JAM_DLL_PUBLIC) %s; // IWYU pragma: keep\n", c->name());
			return false;
		};

		bool
		visitStructStart(const Slice::StructPtr & s) override
		{
			fprintbf(cpp, "struct ICE_CLASS(JAM_DLL_PUBLIC) %s; // IWYU pragma: keep\n", s->name());
			return false;
		};

		void
		visitEnum(const Slice::EnumPtr & e) override
		{
			fprintbf(cpp, "FORWARD_ENUM(%s)\n", e->name());
		};

		void
		visitModuleEnd(const Slice::ModulePtr & m) override
		{
			if (count.classes || count.structs || count.enums) {
				fprintbf(cpp, "} // %s\n\n", m->name());
			}
		}

		FILE * cpp;
		const Count & count;
	};

	SplitString::SplitString(std::string_view in, std::string_view by)
	{
		boost::algorithm::split(*this, in, boost::algorithm::is_any_of(by), boost::algorithm::token_compress_off);
	}

	static std::ostream &
	operator<<(std::ostream & o, const CppName & name)
	{
		for (const auto & s : name) {
			if (&s != &name.front()) {
				o << "::";
			}
			o << s;
		}
		return o;
	}

	Slicer::Slicer() : cpp(nullptr), headerPrefix("slicer"), components(0), classNo(0) { }

	void
	Slicer::defineConversions(const Slice::DataMemberPtr & dm) const
	{
		if (!cpp) {
			return;
		}

		auto type = dm->type();
		auto c = Slice::ContainedPtr::dynamicCast(dm->container());
		auto conversions = getConversions(getAllMetadata(dm));
		for (const auto & conversion : conversions) {
			if (!AdHoc::containerContains(conversion.Options, "nodeclare")) {
				if (!AdHoc::containerContains(conversion.Options, "nodeclareto")) {
					fprintbf(cpp, "DLL_PUBLIC %s %s(const %s &);\n", conversion.ExchangeType,
							conversion.ConvertToExchangeFunc, Slice::typeToString(type));
				}
				if (!AdHoc::containerContains(conversion.Options, "nodeclarefrom")) {
					fprintbf(cpp, "DLL_PUBLIC %s %s(const %s &);\n\n", Slice::typeToString(type),
							conversion.ConvertToModelFunc, conversion.ExchangeType);
				}
			}
		}
		if (!conversions.empty()) {
			fprintbf(cpp, "template<> DLL_PUBLIC\nvoid\n");
			createModelPartForConverted(type, c->scoped(), dm);
			fprintbf(cpp, "::SetValue(ValueSource && vsp)\n{\n");
			fprintbf(cpp, "\t// NOLINTNEXTLINE(hicpp-no-array-decay,-warnings-as-errors)\n");
			fprintbf(cpp, "\tBOOST_ASSERT(Model);\n");

			for (const auto & conversion : conversions) {
				fprintbf(cpp, "\tif (tryConvertFrom< %s >(vsp, Model, &%s)) {\n\t\treturn;\n\t}\n",
						conversion.ExchangeType, conversion.ConvertToModelFunc);
			}
			// Default conversion
			if (!dm->hasMetaData("slicer:nodefaultconversion")) {
				fprintbf(cpp, "\tif (tryConvertFrom< %s >(vsp, Model)) {\n\t\treturn;\n\t}\n",
						Slice::typeToString(type));
			}
			// Failed to convert
			fprintbf(cpp, "\tconversion_fail(\"%s\");\n", Slice::typeToString(type));
			fprintbf(cpp, "}\n\n");

			fprintbf(cpp, "template<> DLL_PUBLIC\nbool\n");
			createModelPartForConverted(type, c->scoped(), dm);
			fprintbf(cpp, "::GetValue(ValueTarget && vtp)\n{\n");
			fprintbf(cpp, "\t// NOLINTNEXTLINE(hicpp-no-array-decay,-warnings-as-errors)\n");
			fprintbf(cpp, "\tBOOST_ASSERT(Model);\n");

			for (const auto & conversion : conversions) {
				fprintbf(cpp,
						"\tif (auto r = tryConvertTo< %s >(vtp, Model, &%s); r != TryConvertResult::NoAction) {\n"
						"\t\treturn (r == TryConvertResult::Value);\n"
						"\t}\n",
						conversion.ExchangeType, conversion.ConvertToExchangeFunc);
			}
			// Default conversion
			if (!dm->hasMetaData("slicer:nodefaultconversion")) {
				fprintbf(cpp,
						"\tif (auto r = tryConvertTo< %s >(vtp, Model); r != TryConvertResult::NoAction) {\n"
						"\t\treturn (r == TryConvertResult::Value);\n"
						"\t}\n",
						Slice::typeToString(type));
			}
			// Failed to convert
			fprintbf(cpp, "\tconversion_fail(\"%s\");\n", Slice::typeToString(type));
			fprintbf(cpp, "}\n\n");

			fprintbf(cpp, "\ttemplate<> DLL_PUBLIC ModelPartPtr ModelPart::Make<");
			createModelPartForConverted(type, c->scoped(), dm);
			fprintbf(cpp, ">(typename ");
			createModelPartForConverted(type, c->scoped(), dm);
			fprintbf(cpp, "::element_type * t) { return std::make_shared<");
			createModelPartForConverted(type, c->scoped(), dm);
			fprintbf(cpp, ">(t); } \n");
		}
	}

	bool
	Slicer::visitUnitStart(const Slice::UnitPtr & u)
	{
		fs::path topLevelFile(u->topLevelFile());
		if (!cpp) {
			return true;
		}
		auto include = [this](const auto & h, bool keep = false) {
			fprintbf(cpp, "#include <%s>", h);
			if (keep) {
				fputs(" // IWYU pragma: keep", cpp);
			}
			fputs("\n", cpp);
		};

		Count count;
		u->visit(&count, true);

		if (count.total == 0) {
			return false;
		}

		fprintbf(cpp, "// Begin Slicer code\n\n");
		include((headerPrefix / "modelPartsTypes.impl.h").string());
		include((headerPrefix / "modelPartsTypes.h").string());
		include((headerPrefix / "modelParts.h").string());
		include((headerPrefix / "hookMap.h").string());
		include((headerPrefix / "metadata.h").string(), true);

		include("array");
		include("optional");
		include("string");
		include("string_view");
		include("visibility.h");
		if (count.classes) {
			include("memory", true);
			include("boost/assert.hpp", true);
			include("Ice/Config.h", true);
		}
		if (count.enums) {
			include((headerPrefix / "enumMap.h").string());
		}
		include("Ice/Optional.h");
		if (count.complexes() || count.dictionaries) {
			include("IceUtil/Config.h");
		}
		if (count.dictionaries) {
			include("map", true);
			include("utility", true);
		}
		ForwardDeclare fd {cpp, count};
		u->visit(&fd, true);

		include(fs::path {topLevelFile.filename()}.replace_extension(".h").string());
		for (const auto & m : u->modules()) {
			IceMetaData md {m->getMetaData()};
			for (const auto & i : md.values("slicer:include:")) {
				include(i);
			}
		}
		fprintbf(cpp, "\n");
		fprintbf(cpp, "namespace Slicer {\n");
		return true;
	}

	void
	Slicer::visitUnitEnd(const Slice::UnitPtr &)
	{
		if (!cpp) {
			return;
		}

		fprintbf(cpp, "}\n\n");
		fprintbf(cpp, "// End Slicer code\n\n");
	}

	bool
	Slicer::visitModuleStart(const Slice::ModulePtr & m)
	{
		if (!cpp) {
			return true;
		}

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
	Slicer::defineRoot(const std::string & type, std::string_view name, const Slice::TypePtr & stype) const
	{
		if (stype->isLocal()) {
			fprintbf(cpp, "template<>\n");
			fprintbf(cpp, "struct isLocal< %s > { static constexpr bool value = true; };\n\n", type);
		}
		fprintbf(cpp, "template<> DLL_PUBLIC\n");
		fprintbf(cpp, "const std::string ModelPartForRoot< %s >::rootName(\"%s\");\n\n", type, name);
		fprintbf(cpp, "template<> DLL_PUBLIC\n");
		fprintbf(cpp, "const std::string ModelPartForRoot< Ice::optional< %s > >::rootName(\"Optional%s\");\n\n", type,
				name);
	}

	void
	Slicer::defineGetMetadata(
			const IceMetaData & md, const Slice::ContainedPtr & t, std::string_view mpt, std::string_view tsuf) const
	{
		fprintbf(cpp, "template<>\nconst Metadata & %s< %s%s >::GetMetadata() const {\n", mpt, t->scoped(), tsuf);
		if (auto n = md.values("slicer:").size()) {
			fprintbf(cpp, "\t\tstatic constexpr MetaDataImpl<%d> md {{{", n);
			copyMetadata(md);
			fprintbf(cpp, "\t\t}}};\n\t\treturn md;\n}\n\n");
		}
		else {
			fprintbf(cpp, "\t\treturn emptyMetadata;\n}\n\n");
		}
	}

	void
	Slicer::externType(const Slice::TypePtr & type) const
	{
		if (definedTypes.count(type->typeId())) {
			return;
		}
		if (Slice::ClassDeclPtr::dynamicCast(type)) {
			fprintbf(cpp, "extern template class ModelPartForComplex< %s >;\n", type->typeId());
		}
		fprintbf(cpp, "extern template class %s< %s >;\n", getBasicModelPart(type),
				Slice::ClassDeclPtr::dynamicCast(type) ? type->typeId() : Slice::typeToString(type));
	}

	bool
	Slicer::visitClassDefStart(const Slice::ClassDefPtr & c)
	{
		if (c->isInterface()) {
			return false;
		}
		if (ignoreType(c)) {
			return false;
		}

		components += 1;

		if (!cpp) {
			return true;
		}

		auto decl = c->declaration();
		fprintbf(cpp, "// Class %s\n", c->name());
		visitComplexDataMembers(decl.get(), c->allDataMembers());

		fprintbf(cpp, "template<>\n");
		const IceMetaData md {c->getMetaData()};
		auto typeId = md.value("slicer:typeid:");
		fprintbf(cpp, "const std::string ModelPartForClass< %s >::typeIdProperty(\"%s\");\n\n", decl->typeId(),
				typeId ? *typeId : "slicer-typeid");

		auto name = md.value("slicer:root:");
		defineRoot(typeToString(decl), name ? *name : c->name(), decl);

		auto typeName = md.value("slicer:typename:");
		fprintbf(cpp, "template<>\n");
		fprintbf(cpp, "const std::string * ModelPartForClass< %s >::className = nullptr;\n", decl->typeId());
		fprintbf(cpp, "template<>\n");
		fprintbf(cpp, "const std::string * ModelPartForClass< %s >::typeName = nullptr;\n", decl->typeId());
		fprintbf(cpp,
				"template<>\nvoid ModelPartForClass< %s >::initClassName() {\n\tclassName = new "
				"std::string(\"%s\");\n\t",
				decl->typeId(), c->scoped());
		if (typeName) {
			fprintbf(cpp, "typeName = new std::string(\"%s\");", *typeName);
		}
		else {
			fprintbf(cpp, "typeName = nullptr;");
		}
		fprintbf(cpp, "\n}\n");

		defineGetMetadata(md, c, "ModelPartForComplex");

		if (auto implementation = md.value("slicer:implementation:")) {
			fprintbf(cpp, "\ttemplate<> void ModelPartForClass<%s>::Create() {\n", c->scoped());
			fprintf(cpp, "\t\tBOOST_ASSERT(this->Model);\n");
			fprintbf(cpp, "\t\t*this->Model = std::make_shared<%s>();\n}\n\n", CppName {*implementation});
		}

		if (auto cmp = md.value("slicer:custommodelpart:")) {
			fprintbf(cpp, "CUSTOMMODELPARTFOR(%s, %s< %s >, %s)\n\n", Slice::typeToString(decl),
					getBasicModelPart(decl), c->scoped(), CppName {*cmp});
			fprintbf(cpp, "\ttemplate<> DLL_PUBLIC ModelPartPtr ModelPart::Make<%s<%s> >(%s * t)",
					getBasicModelPart(decl), c->scoped(), Slice::typeToString(decl));
			fprintbf(cpp, "{ return std::make_shared<%s>(t); } \n", CppName {*cmp});
		}
		else {
			fprintbf(cpp, "CUSTOMMODELPARTFOR(%s, ModelPartForClass<%s>, ModelPartForClass<%s>)\n\n",
					Slice::typeToString(decl), c->scoped(), c->scoped());
		}
		definedTypes.insert(decl->typeId());

		classNo += 1;

		return true;
	}

	bool
	Slicer::visitStructStart(const Slice::StructPtr & c)
	{
		if (ignoreType(c)) {
			return false;
		}

		components += 1;

		if (!cpp) {
			return true;
		}

		fprintbf(cpp, "// Struct %s\n", c->name());
		visitComplexDataMembers(c.get(), c->dataMembers());

		const IceMetaData md {c->getMetaData()};
		auto name = md.value("slicer:root:");
		defineRoot(c->scoped(), name ? *name : c->name(), c);

		defineGetMetadata(md, c, "ModelPartForComplex");

		defineMODELPART(c->scoped(), c, md);

		return true;
	}

	void
	Slicer::visitComplexDataMembers(const Slice::ConstructedPtr & it, const Slice::DataMemberList & dataMembers) const
	{
		if (!cpp) {
			return;
		}

		for (const auto & dm : dataMembers) {
			externType(dm->type());
		}
		fprintbf(cpp, "using C%d = ModelPartForComplex< %s >;\n", components, it->scoped());

		for (const auto & dm : dataMembers) {
			const IceMetaData md {getAllMetadata(dm)};
			auto name = std::string {md.value("slicer:name:").value_or(dm->name())};
			auto lname = std::string {boost::algorithm::to_lower_copy(name)};
			fprintbf(cpp, "\tconst std::string hstr_C%d_%s { \"%s\" };\n", components, dm->name(), name);

			auto c = Slice::ContainedPtr::dynamicCast(dm->container());
			auto t = Slice::TypePtr::dynamicCast(dm->container());
			if (!t) {
				t = Slice::ClassDefPtr::dynamicCast(dm->container())->declaration();
			}
			auto type = dm->type();
			fprintbf(cpp, "\tconstexpr C%d::Hook<", components);
			fprintbf(cpp, " %s, ", Slice::typeToString(type, dm->optional()));
			createNewModelPartPtrFor(type, dm, md);
			fprintbf(cpp, ", %d", md.countSlicerMetaData());
			fprintbf(cpp, R"( > hook_C%d_%s {&%s, "%s", "%s", &hstr_C%d_%s)", components, dm->name(), dm->scoped(),
					name, lname, components, dm->name());
			if (md.hasSlicerMetaData()) {
				fprintbf(cpp, ",");
				copyMetadata(md);
			}
			fprintbf(cpp, "};\n");
		}

		fprintbf(
				cpp, "constexpr const HooksImpl< %s, %d > hooks%d {{{\n", it->scoped(), dataMembers.size(), components);
		for (const auto & dm : dataMembers) {
			fprintbf(cpp, " &hook_C%d_%s,\n", components, dm->name());
		}
		fprintbf(cpp, "\t}}};\n");
		fprintbf(cpp, "\ttemplate<> const Hooks< %s > & C%d::hooks() { return hooks%d; }\n", it->scoped(), components,
				components);
	}

	void
	Slicer::visitEnum(const Slice::EnumPtr & e)
	{
		if (ignoreType(e)) {
			return;
		}

		components += 1;

		if (!cpp) {
			return;
		}

		fprintbf(cpp, "// Enumeration %s\n", e->name());
		const IceMetaData md {e->getMetaData()};
		defineGetMetadata(md, e, "ModelPartForEnum");

		for (const auto & ee : e->enumerators()) {
			fprintbf(cpp, "\tconst std::string estr_E%d_%s { \"%s\" };\n", components, ee->name(), ee->name());
		}
		fprintbf(cpp, "constexpr const EnumMapImpl< %s, %d > enumerations%d {{{\n", e->scoped(),
				e->enumerators().size(), components);
		for (const auto & ee : e->enumerators()) {
			fprintbf(cpp, "\t {%s, \"%s\", &estr_E%d_%s},\n", ee->scoped(), ee->name(), components, ee->name());
		}
		fprintbf(cpp, "\t}}};\n");
		fprintbf(cpp,
				"\ttemplate<> constexpr const EnumMap< %s > & ModelPartForEnum< %s >::enumerations() { return "
				"enumerations%d; }\n",
				e->scoped(), e->scoped(), components);

		auto name = md.value("slicer:root:");
		const Slice::TypePtr t = e;
		defineRoot(e->scoped(), name ? *name : e->name(), t);

		defineMODELPART(e->scoped(), e, md);
	}

	void
	Slicer::visitSequence(const Slice::SequencePtr & s)
	{
		if (ignoreType(s)) {
			return;
		}

		components += 1;

		if (!cpp) {
			return;
		}

		fprintbf(cpp, "// Sequence %s\n", s->name());
		externType(s->type());
		const IceMetaData md {s->getMetaData()};
		auto ename = md.value("slicer:element:");
		fprintbf(cpp, "template<> DLL_PUBLIC\n");
		fprintbf(cpp, "const std::string ModelPartForSequence< %s >::elementName(\"%s\");\n\n", s->scoped(),
				ename ? *ename : "element");

		auto name = md.value("slicer:root:");
		defineRoot(s->scoped(), name ? *name : s->name(), s);

		defineGetMetadata(md, s, "ModelPartForSequence");

		defineMODELPART(s->scoped(), s, md);
	}

	void
	Slicer::visitDictionary(const Slice::DictionaryPtr & d)
	{
		if (ignoreType(d)) {
			return;
		}

		components += 1;

		if (!cpp) {
			return;
		}

		fprintbf(cpp, "// Dictionary %s\n", d->name());
		externType(d->keyType());
		externType(d->valueType());
		const IceMetaData md {d->getMetaData()};
		auto iname = md.value("slicer:item:");
		fprintbf(cpp, "template<>\n");
		fprintbf(cpp, "const std::string ModelPartForDictionary< %s >::pairName(\"%s\");\n\n", d->scoped(),
				iname ? *iname : "element");

		fprintbf(cpp, "using C%d = ModelPartForComplex< %s::value_type >;\n", components, d->scoped());
		auto addHook = [&](std::string_view name, const char * element, const Slice::TypePtr & t, bool cc) {
			auto lname = std::string {name};
			boost::algorithm::to_lower(lname);
			fprintbf(cpp, "\tconst std::string hstr_C%d_%d { \"%s\" };\n", components, element, name);
			fprintbf(cpp, "\tconstexpr C%d::Hook< %s, ", components, Slice::typeToString(t));
			createNewModelPartPtrFor(t);
			fprintbf(cpp, ", 0 > hook%d_%s {", components, element);
			if (cc) {
				fprintbf(cpp, "const_cast<%s (%s::value_type::*)>", Slice::typeToString(t), d->scoped());
			}
			fprintbf(cpp, "(&%s::value_type::%s), \"%s\", \"%s\", &hstr_C%d_%s};\n", d->scoped(), element, name, lname,
					components, element);
		};
		addHook(md.value("slicer:key:").value_or("key"), "first", d->keyType(), true);
		addHook(md.value("slicer:value:").value_or("value"), "second", d->valueType(), false);

		fprintbf(cpp, "constexpr const HooksImpl< %s::value_type, 2 > hooks%d {{{\n", d->scoped(), components);
		fprintbf(cpp, " &hook%d_first,\n", components);
		fprintbf(cpp, " &hook%d_second,\n", components);
		fprintbf(cpp, "\t}}};\n");
		fprintbf(cpp, "\ttemplate<> const Hooks< %s::value_type > & C%d::hooks() { return hooks%d; }\n", d->scoped(),
				components, components);

		auto name = md.value("slicer:root:");
		defineRoot(d->scoped(), name ? *name : d->name(), d);

		defineGetMetadata(md, d, "ModelPartForDictionary");
		defineGetMetadata(md, d, "ModelPartForComplex", "::value_type");

		defineMODELPART(d->scoped(), d, md);
	}

	void
	Slicer::visitModuleEnd(const Slice::ModulePtr & m)
	{
		if (cpp) {
			fprintbf(cpp, "// End module %s\n\n", m->name());
		}
	}

	void
	Slicer::createModelPartForConverted(
			const Slice::TypePtr & type, const std::string & container, const Slice::DataMemberPtr & dm) const
	{
		fprintbf(cpp, "ModelPartForConverted< ");
		if (dm->optional()) {
			fprintbf(cpp, "Ice::optional< %s >", Slice::typeToString(type));
		}
		else {
			fprintbf(cpp, "%s", Slice::typeToString(type));
		}
		fprintbf(cpp, ", %s, &%s >", container, dm->scoped());
	}

	void
	Slicer::createNewModelPartPtrFor(
			const Slice::TypePtr & type, const Slice::DataMemberPtr & dm, const IceMetaData & md) const
	{
		auto conversions = getConversions(md);
		if (dm && !conversions.empty()) {
			createModelPartForConverted(
					type, boost::algorithm::trim_right_copy_if(dm->container()->thisScope(), ispunct), dm);
		}
		else if (auto cmp = md.value("slicer:custommodelpart:")) {
			fprintbf(cpp, "%s", CppName {*cmp});
		}
		else {
			if (dm && dm->optional()) {
				fprintbf(cpp, "ModelPartForOptional< ");
			}
			fprintbf(cpp, "%s< %s >", getBasicModelPart(type),
					Slice::ClassDeclPtr::dynamicCast(type) ? type->typeId() : Slice::typeToString(type));
			if (dm && dm->optional()) {
				fprintbf(cpp, " > ");
			}
		}
	}

	std::string
	Slicer::getBasicModelPart(const Slice::TypePtr & type) const
	{
#define CheckTypeAndReturn(Type, ModerlPartClass) \
	if (Type::dynamicCast(type)) { \
		return ModerlPartClass; \
	}
		CheckTypeAndReturn(Slice::BuiltinPtr, "ModelPartForSimple");
		CheckTypeAndReturn(Slice::ClassDeclPtr, "ModelPartForClass");
		CheckTypeAndReturn(Slice::StructPtr, "ModelPartForStruct");
		CheckTypeAndReturn(Slice::SequencePtr, "ModelPartForSequence");
		CheckTypeAndReturn(Slice::DictionaryPtr, "ModelPartForDictionary");
		CheckTypeAndReturn(Slice::EnumPtr, "ModelPartForEnum");
#undef CheckTypeAndReturn
		throw CompilerError("Unknown basic type");
	}

	void
	Slicer::copyMetadata(const IceMetaData & metadata) const
	{
		for (const auto & md : metadata.getSlicerMetaData()) {
			fprintbf(cpp, "\t\"%s\",\n", md);
		}
	}

	IceMetaData
	Slicer::getAllMetadata(const Slice::DataMemberPtr & dm)
	{
		auto metadata = dm->getMetaData();
		auto typec = Slice::ContainedPtr::dynamicCast(dm->type());
		if (typec) {
			if (auto cd = Slice::ClassDeclPtr::dynamicCast(typec)) {
				typec = cd->definition();
			}
			if (typec) {
				metadata.merge(typec->getMetaData());
			}
		}
		return IceMetaData {std::move(metadata)};
	}

	Slicer::Conversions
	Slicer::getConversions(const IceMetaData & md)
	{
		Conversions rtn;
		auto conversions = md.values("slicer:conversion:");
		for (const auto & conversion : conversions) {
			auto split = IceMetaData::split(conversion);
			if (split.size() < 3) {
				throw CompilerError("conversion needs at least 3 parts type:toModelFunc:toExchangeFunc[:options]");
			}
			rtn.push_back(ConversionSpec {
					CppName {split[0]}, CppName {split[1]}, CppName {split[2]}, {split.begin() + 3, split.end()}});
		}
		return rtn;
	}

	void
	Slicer::defineMODELPART(const std::string & type, const Slice::TypePtr & stype, const IceMetaData & metadata)
	{
		if (auto cmp = metadata.value("slicer:custommodelpart:")) {
			fprintbf(cpp, "CUSTOMMODELPARTFOR(%s, %s< %s >, %s)\n\n", type, getBasicModelPart(stype), type,
					CppName {*cmp});
			fprintbf(cpp, "\ttemplate<> DLL_PUBLIC ModelPartPtr ModelPart::Make<%s<%s>>(%s * t)",
					getBasicModelPart(stype), type, type);
			fprintbf(cpp, "{ return std::make_shared<%s>(t); } \n", CppName {*cmp});
		}
		else {
			fprintbf(cpp, "MODELPARTFOR(%s, %s)\n\n", type, getBasicModelPart(stype));
		}
		definedTypes.insert(stype->typeId());
	}

	unsigned int
	Slicer::Components() const
	{
		return components;
	}

	unsigned int
	Slicer::Execute()
	{
		if (cpp && !cppPath.empty()) {
			throw CompilerError("Both file handle and path provided.");
		}
		auto cppfile = std::unique_ptr<FILE, decltype(&fclose)>(
				cpp || cppPath.empty() ? cpp : fopen(cppPath.c_str(), "w"), cppPath.empty() ? fflush : fclose);
		if (!cppfile && !cppPath.empty()) {
			throw CompilerError("Failed to open output file");
		}
		try {
			cpp = cppfile.get();
			Slicer::Slicer::Args args;
			// Copy includes to args
			std::transform(includes.begin(), includes.end(), std::back_inserter(args), [](auto && include) {
				return "-I" + include.string();
			});

			Slice::PreprocessorPtr icecpp = Slice::Preprocessor::create("slicer", slicePath, args);
			FILE * cppHandle = icecpp->preprocess(false);

			if (!cppHandle) {
				throw CompilerError("preprocess failed");
			}

			Slice::UnitPtr u = Slice::Unit::createUnit(false, false, false, false);

			int parseStatus = u->parse(slicePath, cppHandle, false);

			if (!icecpp->close()) {
				u->destroy();
				throw CompilerError("preprocess close failed");
			}

			if (parseStatus == EXIT_FAILURE) {
				u->destroy();
				throw CompilerError("unit parse failed");
			}

			unsigned int initial = components;

			u->visit(this, false);

			u->destroy();

			if (!cppPath.empty()) {
				cpp = nullptr;
			}

			return components - initial;
		}
		catch (...) {
			if (!cppPath.empty()) {
				std::filesystem::remove(cppPath);
			}
			throw;
		}
	}
}
