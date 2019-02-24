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
					ConversionSpec(const Args &);

					std::string ExchangeType;
					std::string ConvertToModelFunc;
					std::string ConvertToExchangeFunc;
					Args Options;
			};
			typedef std::vector<ConversionSpec> Conversions;

			Slicer();

			FILE * cpp;
			std::filesystem::path slicePath;
			std::filesystem::path cppPath;
			std::filesystem::path headerPrefix;
			std::vector<std::filesystem::path> includes;

			unsigned int Execute();
			unsigned int Components() const;

#pragma GCC visibility push(hidden)
			virtual bool visitUnitStart(const Slice::UnitPtr&) override;

			virtual void visitUnitEnd(const Slice::UnitPtr&) override;

			virtual bool visitModuleStart(const Slice::ModulePtr & m) override;

			virtual bool visitClassDefStart(const Slice::ClassDefPtr & c) override;

			virtual bool visitStructStart(const Slice::StructPtr&) override;

			virtual void visitEnum(const Slice::EnumPtr &) override;

			virtual void visitSequence(const Slice::SequencePtr & s) override;

			virtual void visitDictionary(const Slice::DictionaryPtr & d) override;

			virtual void visitModuleEnd(const Slice::ModulePtr & m) override;

		private:
			void createModelPartForConverted(const Slice::TypePtr & type, const std::string & container, const Slice::DataMemberPtr & dm) const;
			void createNewModelPartPtrFor(const Slice::TypePtr & type, const Slice::DataMemberPtr & dm = Slice::DataMemberPtr(), const Slice::StringList & md = Slice::StringList()) const;
			std::string getBasicModelPart(const Slice::TypePtr & type) const;
			void defineMODELPART(const std::string & type, const Slice::TypePtr & stype, const Slice::StringList & metadata) const;

			void visitComplexDataMembers(const Slice::ConstructedPtr & t, const Slice::DataMemberList &) const;

			void defineConversions(const Slice::DataMemberPtr & dm) const;
			void defineRoot(const std::string & type, const std::string & name, const Slice::TypePtr & stype) const;

			bool hasMetadata(const std::list<std::string> & metadata) const;
			void copyMetadata(const std::list<std::string> & metadata) const;
			static Slice::StringList getAllMetadata(const Slice::DataMemberPtr & dm);
			static Conversions getConversions(const Slice::StringList & metadata);
#pragma GCC visibility pop

			unsigned int components;
			unsigned int classNo;
	};
}

#endif

