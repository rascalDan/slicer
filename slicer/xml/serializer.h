#ifndef SLICER_XML_H
#define SLICER_XML_H

#include <slicer/serializer.h>
#include <libxml++/document.h>
#include <visibility.h>

namespace Slicer {
	class DLL_PUBLIC XmlSerializer : public Serializer {
		protected:
			typedef boost::function<xmlpp::Element *(xmlpp::Element *, const Glib::ustring &)> ElementCreator;
			static void ModelTreeIterate(xmlpp::Element *, const std::string &, ModelPartPtr mp, HookCommonPtr hp, const ElementCreator &);
			static void ModelTreeIterateRoot(xmlpp::Document *, const std::string &, ModelPartPtr mp);

		protected:
			static void ModelTreeProcessElement(xmlpp::Element * n, ModelPartPtr mp, const ElementCreator &);
	};

	class DLL_PUBLIC XmlStreamSerializer : public XmlSerializer {
		public:
			XmlStreamSerializer(std::ostream &);

			virtual void Serialize(ModelPartForRootPtr) override;

		protected:
			std::ostream & strm;
	};

	class DLL_PUBLIC XmlFileSerializer : public XmlSerializer {
		public:
			XmlFileSerializer(const boost::filesystem::path &);

			virtual void Serialize(ModelPartForRootPtr) override;

		protected:
			const boost::filesystem::path path;
	};

	class DLL_PUBLIC XmlDocumentSerializer : public XmlSerializer {
		public:
			XmlDocumentSerializer(xmlpp::Document * &);

			virtual void Serialize(ModelPartForRootPtr) override;

		protected:
			xmlpp::Document * & doc;
	};

	class DLL_PUBLIC XmlDeserializer : public Deserializer {
		protected:
			static void DocumentTreeIterate(const xmlpp::Node * node, ModelPartPtr mp);
			static void DocumentTreeIterate(const xmlpp::Document * doc, ModelPartPtr mp);
	};

	class DLL_PUBLIC XmlStreamDeserializer : public XmlDeserializer {
		public:
			XmlStreamDeserializer(std::istream &);

			virtual void Deserialize(ModelPartForRootPtr) override;

		protected:
			std::istream & strm;
	};

	class DLL_PUBLIC XmlFileDeserializer : public XmlDeserializer {
		public:
			XmlFileDeserializer(const boost::filesystem::path &);

			virtual void Deserialize(ModelPartForRootPtr) override;

		protected:
			const boost::filesystem::path path;
	};

	class DLL_PUBLIC XmlDocumentDeserializer : public XmlDeserializer {
		public:
			XmlDocumentDeserializer(const xmlpp::Document *);

			virtual void Deserialize(ModelPartForRootPtr) override;

		protected:
			const xmlpp::Document * doc;
	};
}

#endif

