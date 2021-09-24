#ifndef SLICER_PARSER_H
#define SLICER_PARSER_H

#include "icemetadata.h"
#include <Slice/Parser.h>
#include <cstdio>
#include <filesystem>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <visibility.h>
// IWYU pragma: no_include <boost/iterator/transform_iterator.hpp>

namespace Slicer {
	class SplitString : public std::vector<std::string> {
	public:
		SplitString(std::string_view in, std::string_view split);
		using std::vector<std::string>::vector;
	};

	struct CppName : public SplitString {
		explicit inline CppName(std::string_view in) : SplitString {in, "."} { }
	};

	class DLL_PUBLIC Slicer : public Slice::ParserVisitor {
	public:
		struct Args : public SplitString {
			explicit inline Args(std::string_view in) : SplitString {in, ","} { }
			using SplitString::SplitString;
		};

		struct ConversionSpec {
			CppName ExchangeType;
			CppName ConvertToModelFunc;
			CppName ConvertToExchangeFunc;
			Args Options;
		};

		using Conversions = std::vector<ConversionSpec>;

		Slicer();

		FILE * cpp;
		std::filesystem::path slicePath;
		std::filesystem::path cppPath;
		std::filesystem::path headerPrefix;
		std::vector<std::filesystem::path> includes;

		unsigned int Execute();
		[[nodiscard]] unsigned int Components() const;

#pragma GCC visibility push(hidden)
		bool visitUnitStart(const Slice::UnitPtr &) override;

		void visitUnitEnd(const Slice::UnitPtr &) override;

		bool visitModuleStart(const Slice::ModulePtr & m) override;

		bool visitClassDefStart(const Slice::ClassDefPtr & c) override;

		bool visitStructStart(const Slice::StructPtr &) override;

		void visitEnum(const Slice::EnumPtr &) override;

		void visitSequence(const Slice::SequencePtr & s) override;

		void visitDictionary(const Slice::DictionaryPtr & d) override;

		void visitModuleEnd(const Slice::ModulePtr & m) override;

	private:
		void createModelPartForConverted(
				const Slice::TypePtr & type, const std::string & container, const Slice::DataMemberPtr & dm) const;
		void createNewModelPartPtrFor(const Slice::TypePtr & type, const Slice::DataMemberPtr & dm = {},
				const IceMetaData & md = IceMetaData {}) const;
		[[nodiscard]] std::string getBasicModelPart(const Slice::TypePtr & type) const;
		void defineMODELPART(const std::string & type, const Slice::TypePtr & stype, const IceMetaData & metadata);

		void visitComplexDataMembers(const Slice::ConstructedPtr & t, const Slice::DataMemberList &) const;

		void defineConversions(const Slice::DataMemberPtr & dm) const;
		void defineRoot(const std::string & type, std::string_view name, const Slice::TypePtr & stype) const;
		void defineGetMetadata(const IceMetaData &, const Slice::ContainedPtr &, std::string_view mpt,
				std::string_view tsuf = {}) const;
		void externType(const Slice::TypePtr &) const;

		void copyMetadata(const IceMetaData & metadata) const;
		static IceMetaData getAllMetadata(const Slice::DataMemberPtr & dm);
		static Conversions getConversions(const IceMetaData & metadata);
		std::set<std::string> definedTypes;
#pragma GCC visibility pop

		unsigned int components;
		unsigned int classNo;
	};
}

#endif
