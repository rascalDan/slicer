#ifndef SLICER_XML_H
#define SLICER_XML_H

#include <slicer/serializer.h>
#include <libxml++/document.h>

namespace Slicer {
	class XmlSerializer : public Serializer {
		protected:
			static void ModelTreeIterate(xmlpp::Element *, const std::string &, ModelPartPtr mp);
			static void ModelTreeIterateRoot(xmlpp::Document *, const std::string &, ModelPartPtr mp);
	};
	
	class XmlFileSerializer : public XmlSerializer {
		public:
			XmlFileSerializer(const boost::filesystem::path &);

			virtual void Serialize(ModelPartPtr) override;

		private:
			const boost::filesystem::path path;
	};
	
	class XmlDocumentSerializer : public XmlSerializer {
		public:
			XmlDocumentSerializer(xmlpp::Document * &);

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
			XmlFileDeserializer(const boost::filesystem::path &);

			virtual void Deserialize(ModelPartPtr) override;

		private:
			const boost::filesystem::path path;
	};
	
	class XmlDocumentDeserializer : public XmlDeserializer {
		public:
			XmlDocumentDeserializer(const xmlpp::Document *);

			virtual void Deserialize(ModelPartPtr) override;

		private:
			const xmlpp::Document * doc;
	};
}

#endif

