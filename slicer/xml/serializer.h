#ifndef SLICER_XML_H
#define SLICER_XML_H

#include <lazyPointer.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#ifndef __clang__
#	pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
#include <libxml++/document.h>
#include <libxml++/nodes/element.h>
#pragma GCC diagnostic pop
#include <filesystem>
#include <fstream>
#include <functional>
#include <iosfwd>
#include <slicer/modelParts.h>
#include <slicer/serializer.h>
#include <string>
#include <visibility.h>

namespace Glib {
	class ustring;
}

namespace Slicer {
	using CurrentElementCreator = ::AdHoc::LazyPointer<xmlpp::Element, xmlpp::Element *>;

	class DLL_PUBLIC XmlSerializer : public Serializer {
	protected:
		using ElementCreator = std::function<xmlpp::Element *(xmlpp::Element *, const Glib::ustring &)>;
		static void ModelTreeIterate(xmlpp::Element *, const std::string &, ModelPartParam mp, const HookCommon * hp,
				const ElementCreator &);
		static void ModelTreeIterateRoot(xmlpp::Document *, const std::string &, ModelPartParam mp);

	protected:
		static void ModelTreeProcessElement(const CurrentElementCreator &, ModelPartParam mp, const ElementCreator &);
		static void ModelTreeIterateDictAttrs(xmlpp::Element * element, ModelPartParam dict);
		static void ModelTreeIterateDictElements(xmlpp::Element * element, ModelPartParam dict);
	};

	class DLL_PUBLIC XmlDocumentSerializer : public XmlSerializer {
	public:
		void Serialize(ModelPartForRootPtr) override;

	protected:
		xmlpp::Document doc;
	};

	class DLL_PUBLIC XmlStreamSerializer : public XmlDocumentSerializer {
	public:
		explicit XmlStreamSerializer(std::ostream &);

		void Serialize(ModelPartForRootPtr) override;

	protected:
		std::ostream & strm;
	};

	class DLL_PUBLIC XmlFileSerializer : public XmlStreamSerializer {
	public:
		explicit XmlFileSerializer(const std::filesystem::path &);

	protected:
		std::ofstream strm;
	};

	class DLL_PUBLIC XmlDeserializer : public Deserializer {
	protected:
		static void DocumentTreeIterate(const xmlpp::Node * node, ModelPartParam mp);
		static void DocumentTreeIterateElement(const xmlpp::Element * element, ModelPartParam mp, const ChildRef & c);
		static void DocumentTreeIterate(const xmlpp::Document * doc, ModelPartParam mp);
		static void DocumentTreeIterateDictAttrs(
				const xmlpp::Element::const_AttributeList & attrs, ModelPartParam dict);
		static void DocumentTreeIterateDictElements(const xmlpp::Element * parent, ModelPartParam dict);
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
