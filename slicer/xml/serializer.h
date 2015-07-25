#ifndef SLICER_XML_H
#define SLICER_XML_H

#include <slicer/serializer.h>
#include <libxml++/document.h>

#ifndef DLL_PUBLIC
#define DLL_PUBLIC __attribute__ ((visibility ("default")))
#endif

namespace Slicer {
	class XmlSerializer : public Serializer {
		protected:
			typedef boost::function<xmlpp::Element *(xmlpp::Element *, const Glib::ustring &)> ElementCreator;
			static void ModelTreeIterate(xmlpp::Element *, const std::string &, ModelPartPtr mp, HookCommonPtr hp, const ElementCreator &);
			static void ModelTreeIterateRoot(xmlpp::Document *, const std::string &, ModelPartPtr mp);

		private:
			static void ModelTreeProcessElement(xmlpp::Element * n, ModelPartPtr mp, const ElementCreator &);
	};

	class XmlFileSerializer : public XmlSerializer {
		public:
			DLL_PUBLIC XmlFileSerializer(const boost::filesystem::path &);

			virtual void Serialize(ModelPartPtr) override;

		private:
			const boost::filesystem::path path;
	};

	class XmlDocumentSerializer : public XmlSerializer {
		public:
			DLL_PUBLIC XmlDocumentSerializer(xmlpp::Document * &);

			virtual void Serialize(ModelPartPtr) override;

		private:
			xmlpp::Document * & doc;
	};

	class XmlDeserializer : public Deserializer {
		protected:
			static void DocumentTreeIterate(const xmlpp::Node * node, ModelPartPtr mp);
			static void DocumentTreeIterate(const xmlpp::Document * doc, ModelPartPtr mp);
	};

	class XmlFileDeserializer : public XmlDeserializer {
		public:
			DLL_PUBLIC XmlFileDeserializer(const boost::filesystem::path &);

			virtual void Deserialize(ModelPartPtr) override;

		private:
			const boost::filesystem::path path;
	};

	class XmlDocumentDeserializer : public XmlDeserializer {
		public:
			DLL_PUBLIC XmlDocumentDeserializer(const xmlpp::Document *);

			virtual void Deserialize(ModelPartPtr) override;

		private:
			const xmlpp::Document * doc;
	};
}

#endif

