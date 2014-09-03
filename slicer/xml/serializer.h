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
			Xml(const boost::filesystem::path &);

			virtual void Deserialize(ModelPartPtr) override;
			virtual void Serialize(ModelPartPtr) override;

		protected:
			static void DocumentTreeIterate(const xmlpp::Node * node, ModelPartPtr mp);
			static void DocumentTreeIterate(const xmlpp::Document * doc, ModelPartPtr mp);
			static void ModelTreeIterate(xmlpp::Element *, const std::string &, ModelPartPtr mp);
			static void ModelTreeIterateRoot(xmlpp::Document *, const std::string &, ModelPartPtr mp);

		private:
			const boost::filesystem::path path;
	};
}

#endif

