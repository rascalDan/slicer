#ifndef SLICER_XML_H
#define SLICER_XML_H

#include <slicer/serializer.h>
#include <libxml++/document.h>

namespace Slicer {
	class Xml : public Serializer {
		protected:
			static void DocumentTreeIterate(const xmlpp::Node * node, ModelPartPtr mp);
			static void DocumentTreeIterate(const xmlpp::Document * doc, ModelPartPtr mp);
			static void ModelTreeIterate(xmlpp::Element *, const std::string &, ModelPartPtr mp);
			static void ModelTreeIterateRoot(xmlpp::Document *, const std::string &, ModelPartPtr mp);
	};
	
	class XmlFile : public Xml {
		public:
			XmlFile(const boost::filesystem::path &);

			virtual void Deserialize(ModelPartPtr) override;
			virtual void Serialize(ModelPartPtr) override;

		private:
			const boost::filesystem::path path;
	};
	
	class XmlDocument : public Xml {
		public:
			XmlDocument(xmlpp::Document * &);

			virtual void Deserialize(ModelPartPtr) override;
			virtual void Serialize(ModelPartPtr) override;

		private:
			xmlpp::Document * & doc;
	};
}

#endif

