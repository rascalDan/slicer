#ifndef SLICER_PARSER_H
#define SLICER_PARSER_H

#include <Slice/Parser.h>
#include <filesystem>
#include <visibility.h>

namespace Slicer {
	class DLL_PUBLIC Slicer : public Slice::ParserVisitor {
		public:
			typedef std::vector<std::string> Args;
			class ConversionSpec {
				public:
					explicit ConversionSpec(const Args &);

					// NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
					std::string ExchangeType;
					// NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
					std::string ConvertToModelFunc;
					// NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
					std::string ConvertToExchangeFunc;
					// NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
					Args Options;
			};
			using Conversions = std::vector<ConversionSpec>;

			Slicer();

			// NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
			FILE * cpp;
			// NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
			std::filesystem::path slicePath;
			// NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
			std::filesystem::path cppPath;
			// NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
			std::filesystem::path headerPrefix;
			// NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
			std::vector<std::filesystem::path> includes;

			unsigned int Execute();
			[[nodiscard]] unsigned int Components() const;

#pragma GCC visibility push(hidden)
			bool visitUnitStart(const Slice::UnitPtr&) override;

			void visitUnitEnd(const Slice::UnitPtr&) override;

			bool visitModuleStart(const Slice::ModulePtr & m) override;

			bool visitClassDefStart(const Slice::ClassDefPtr & c) override;

			bool visitStructStart(const Slice::StructPtr&) override;

			void visitEnum(const Slice::EnumPtr &) override;

			void visitSequence(const Slice::SequencePtr & s) override;

			void visitDictionary(const Slice::DictionaryPtr & d) override;

			void visitModuleEnd(const Slice::ModulePtr & m) override;

		private:
			void createModelPartForConverted(const Slice::TypePtr & type, const std::string & container, const Slice::DataMemberPtr & dm) const;
			void createNewModelPartPtrFor(const Slice::TypePtr & type, const Slice::DataMemberPtr & dm = Slice::DataMemberPtr(), const Slice::StringList & md = Slice::StringList()) const;
			[[nodiscard]] std::string getBasicModelPart(const Slice::TypePtr & type) const;
			void defineMODELPART(const std::string & type, const Slice::TypePtr & stype, const Slice::StringList & metadata);

			void visitComplexDataMembers(const Slice::ConstructedPtr & t, const Slice::DataMemberList &) const;

			void defineConversions(const Slice::DataMemberPtr & dm) const;
			void defineRoot(const std::string & type, const std::string & name, const Slice::TypePtr & stype) const;
			void externType(const Slice::TypePtr &) const;

			[[nodiscard]] bool hasMetadata(const std::list<std::string> & metadata) const;
			void copyMetadata(const std::list<std::string> & metadata) const;
			static Slice::StringList getAllMetadata(const Slice::DataMemberPtr & dm);
			static Conversions getConversions(const Slice::StringList & metadata);
			std::set<std::string> definedTypes;
#pragma GCC visibility pop

			unsigned int components;
			unsigned int classNo;
	};
}

#endif

