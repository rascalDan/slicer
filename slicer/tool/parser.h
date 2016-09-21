#ifndef SLICER_PARSER_H
#define SLICER_PARSER_H

#include <Slice/Parser.h>
#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>
#include <visibility.h>

namespace Slicer {
	typedef boost::shared_ptr<FILE> FilePtr;

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
			boost::filesystem::path slicePath;
			boost::filesystem::path cppPath;
			boost::filesystem::path headerPrefix;
			std::vector<boost::filesystem::path> includes;
			bool allowIcePrefix;

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
			void createNewModelPartPtrFor(const Slice::TypePtr & type) const;

			void visitComplexDataMembers(Slice::ConstructedPtr t, const Slice::DataMemberList &) const;

			void defineConversions(Slice::DataMemberPtr dm) const;
			void defineRootName(const std::string & type, const std::string & name) const;

			bool hasMetadata(const std::list<std::string> & metadata) const;
			void copyMetadata(const std::list<std::string> & metadata) const;
			static Conversions getAllConversions(Slice::DataMemberPtr dm);
			static Conversions getConversions(const std::list<std::string> & metadata);
#pragma GCC visibility pop

			unsigned int components;
			unsigned int classNo;
	};
}

#endif

