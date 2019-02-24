#ifndef SLICER_XML_H
#define SLICER_XML_H

#include <slicer/serializer.h>
#include <libxml++/document.h>
#include <libxml++/nodes/element.h>
#include <visibility.h>
#include <lazyPointer.h>

namespace Slicer {
	typedef ::AdHoc::LazyPointer<xmlpp::Element, xmlpp::Element *> CurrentElementCreator;

	class DLL_PUBLIC XmlSerializer : public Serializer {
		protected:
			typedef std::function<xmlpp::Element *(xmlpp::Element *, const Glib::ustring &)> ElementCreator;
			static void ModelTreeIterate(xmlpp::Element *, const std::string &, const ModelPartPtr & mp, const HookCommon * hp, const ElementCreator &);
			static void ModelTreeIterateRoot(xmlpp::Document *, const std::string &, const ModelPartPtr & mp);

		protected:
			static void ModelTreeProcessElement(const CurrentElementCreator &, ModelPartPtr mp, const ElementCreator &);
			static void ModelTreeIterateDictAttrs(xmlpp::Element * element, const ModelPartPtr & dict);
			static void ModelTreeIterateDictElements(xmlpp::Element * element, const ModelPartPtr & dict);
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
			XmlFileSerializer(std::filesystem::path);

			virtual void Serialize(ModelPartForRootPtr) override;

		protected:
			const std::filesystem::path path;
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
			static void DocumentTreeIterate(const xmlpp::Node * node, const ModelPartPtr & mp);
			static void DocumentTreeIterateElement(const xmlpp::Element * element, ModelPartPtr mp, const ChildRef & c);
			static void DocumentTreeIterate(const xmlpp::Document * doc, const ModelPartPtr & mp);
			static void DocumentTreeIterateDictAttrs(const xmlpp::Element::const_AttributeList & attrs, const ModelPartPtr & dict);
			static void DocumentTreeIterateDictElements(const xmlpp::Element * parent, const ModelPartPtr & dict);
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
			XmlFileDeserializer(std::filesystem::path);

			virtual void Deserialize(ModelPartForRootPtr) override;

		protected:
			const std::filesystem::path path;
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

