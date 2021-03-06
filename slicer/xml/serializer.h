#ifndef SLICER_XML_H
#define SLICER_XML_H

#include <lazyPointer.h>
#include <libxml++/document.h>
#include <libxml++/nodes/element.h>
#include <slicer/serializer.h>
#include <visibility.h>

namespace Slicer {
	using CurrentElementCreator = ::AdHoc::LazyPointer<xmlpp::Element, xmlpp::Element *>;

	class DLL_PUBLIC XmlSerializer : public Serializer {
	protected:
		using ElementCreator = std::function<xmlpp::Element *(xmlpp::Element *, const Glib::ustring &)>;
		static void ModelTreeIterate(xmlpp::Element *, const std::string &, const ModelPartPtr & mp,
				const HookCommon * hp, const ElementCreator &);
		static void ModelTreeIterateRoot(xmlpp::Document *, const std::string &, const ModelPartPtr & mp);

	protected:
		static void ModelTreeProcessElement(const CurrentElementCreator &, ModelPartPtr mp, const ElementCreator &);
		static void ModelTreeIterateDictAttrs(xmlpp::Element * element, const ModelPartPtr & dict);
		static void ModelTreeIterateDictElements(xmlpp::Element * element, const ModelPartPtr & dict);
	};

	class DLL_PUBLIC XmlStreamSerializer : public XmlSerializer {
	public:
		explicit XmlStreamSerializer(std::ostream &);

		void Serialize(ModelPartForRootPtr) override;

	protected:
		std::ostream & strm;
	};

	class DLL_PUBLIC XmlFileSerializer : public XmlSerializer {
	public:
		explicit XmlFileSerializer(std::filesystem::path);

		void Serialize(ModelPartForRootPtr) override;

	protected:
		const std::filesystem::path path;
	};

	class DLL_PUBLIC XmlDocumentSerializer : public XmlSerializer {
	public:
		explicit XmlDocumentSerializer(xmlpp::Document *&);

		void Serialize(ModelPartForRootPtr) override;

	protected:
		xmlpp::Document *& doc;
	};

	class DLL_PUBLIC XmlDeserializer : public Deserializer {
	protected:
		static void DocumentTreeIterate(const xmlpp::Node * node, const ModelPartPtr & mp);
		static void DocumentTreeIterateElement(const xmlpp::Element * element, ModelPartPtr mp, const ChildRef & c);
		static void DocumentTreeIterate(const xmlpp::Document * doc, const ModelPartPtr & mp);
		static void DocumentTreeIterateDictAttrs(
				const xmlpp::Element::const_AttributeList & attrs, const ModelPartPtr & dict);
		static void DocumentTreeIterateDictElements(const xmlpp::Element * parent, const ModelPartPtr & dict);
	};

	class DLL_PUBLIC XmlStreamDeserializer : public XmlDeserializer {
	public:
		explicit XmlStreamDeserializer(std::istream &);

		void Deserialize(ModelPartForRootPtr) override;

	protected:
		std::istream & strm;
	};

	class DLL_PUBLIC XmlFileDeserializer : public XmlDeserializer {
	public:
		explicit XmlFileDeserializer(std::filesystem::path);

		void Deserialize(ModelPartForRootPtr) override;

	protected:
		const std::filesystem::path path;
	};

	class DLL_PUBLIC XmlDocumentDeserializer : public XmlDeserializer {
	public:
		explicit XmlDocumentDeserializer(const xmlpp::Document *);

		void Deserialize(ModelPartForRootPtr) override;

	protected:
		const xmlpp::Document * doc;
	};
}

#endif
