#ifndef SLICER_PARSER_H
#define SLICER_PARSER_H

#include <Slice/Parser.h>
#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>

namespace Slicer {
	typedef boost::shared_ptr<FILE> FilePtr;

	class Slicer : public Slice::ParserVisitor {
		public:
			Slicer(FILE * c);

			static void Apply(const boost::filesystem::path & ice, const boost::filesystem::path & cpp);

			void leadIn();

			void leadOut();

			virtual bool visitModuleStart(const Slice::ModulePtr & m) override;

			virtual bool visitClassDefStart(const Slice::ClassDefPtr & c) override;

			virtual bool visitStructStart(const Slice::StructPtr&) override;

			void visitSequence(const Slice::SequencePtr & s) override;

			void visitDictionary(const Slice::DictionaryPtr & d) override;

			virtual void visitModuleEnd(const Slice::ModulePtr & m) override;


		private:
			void createNewModelPartPtrFor(const Slice::TypePtr & type) const;

			std::string modulePath();

			static boost::optional<std::string> metaDataValue(const std::string & prefix, const std::list<std::string> & metadata);

			FILE * cpp;
			std::vector<Slice::ModulePtr> modules;
	};
}

#endif

