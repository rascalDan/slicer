#ifndef SLICER_XML_H
#define SLICER_XML_H

#include <slicer/serializer.h>

namespace xmlpp {
	class Document;
	class Node;
	class Element;
	class Attribute;
	class ContentNode;
}

namespace Slicer {
	class Xml : public Serializer {
		public:
			virtual void Deserialize(const boost::filesystem::path &, ModelPartPtr) override;
			virtual void Serialize(const boost::filesystem::path &, ModelPartPtr) override;

		protected:
			static void DocumentTreeIterate(const xmlpp::Node * node, ModelPartPtr mp);
			static void DocumentTreeIterate(const xmlpp::Document * doc, ModelPartPtr mp);
			static void ModelTreeIterate(xmlpp::Element *, const std::string &, ModelPartPtr mp);
			static void ModelTreeIterateRoot(xmlpp::Document *, const std::string &, ModelPartPtr mp);
	};
}

#endif

