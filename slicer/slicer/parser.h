#ifndef SLICER_PARSER_H
#define SLICER_PARSER_H

#include <Slice/Parser.h>
#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>

namespace Slicer {
	typedef boost::shared_ptr<FILE> FilePtr;

	class Slicer : public Slice::ParserVisitor {
		public:
			class ConversionSpec {
				public:
					std::string ExchangeType;
					std::string ConvertToModelFunc;
					std::string ConvertToExchangeFunc;
			};

			Slicer(FILE * c);

			static unsigned int Apply(const boost::filesystem::path & ice, const boost::filesystem::path & cpp);
			static unsigned int Apply(const boost::filesystem::path & ice, FILE *);

			virtual bool visitUnitStart(const Slice::UnitPtr&) override;

			virtual void visitUnitEnd(const Slice::UnitPtr&) override;

			virtual bool visitModuleStart(const Slice::ModulePtr & m) override;

			virtual bool visitClassDefStart(const Slice::ClassDefPtr & c) override;

			virtual bool visitStructStart(const Slice::StructPtr&) override;

			void visitSequence(const Slice::SequencePtr & s) override;

			void visitDictionary(const Slice::DictionaryPtr & d) override;

			virtual void visitModuleEnd(const Slice::ModulePtr & m) override;

			unsigned int Components() const;

		private:
			void createNewModelPartPtrFor(const Slice::TypePtr & type) const;

			void visitComplexDataMembers(Slice::ConstructedPtr t, const Slice::DataMemberList &) const;

			void defineConversions(Slice::DataMemberPtr dm) const;

			void copyMetadata(const std::list<std::string> & metadata) const;
			static boost::optional<std::string> metaDataValue(const std::string & prefix, const std::list<std::string> & metadata);
			static std::list<std::string> metaDataValues(const std::string & prefix, const std::list<std::string> & metadata);
			static std::vector<std::string> metaDataSplit(const std::string & metadata);
			static std::vector<ConversionSpec> getConversions(Slice::DataMemberPtr);

			unsigned int components;
			FILE * cpp;
			std::vector<Slice::ModulePtr> modules;
			unsigned int classNo;
	};
}

#endif

