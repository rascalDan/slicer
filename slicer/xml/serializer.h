#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#ifndef __clang__
#	pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
#include <libxml++/document.h>
#pragma GCC diagnostic pop
#include <filesystem>
#include <fstream>
#include <slicer/modelParts.h>
#include <slicer/serializer.h>
#include <visibility.h>

namespace Glib {
	class ustring;
}

namespace Slicer {
	class DLL_PUBLIC XmlDocumentSerializer : public Serializer {
	public:
		void Serialize(ModelPartForRootParam) override;

	protected:
		xmlpp::Document doc;
	};

	class DLL_PUBLIC XmlStreamSerializer : public XmlDocumentSerializer {
	public:
		explicit XmlStreamSerializer(std::ostream &);

		void Serialize(ModelPartForRootParam) override;

	protected:
		std::ostream & strm;
	};

	class DLL_PUBLIC XmlFileSerializer : public XmlStreamSerializer {
	public:
		explicit XmlFileSerializer(const std::filesystem::path &);

	protected:
		std::ofstream strm;
	};

	class DLL_PUBLIC XmlStreamDeserializer : public Deserializer {
	public:
		explicit XmlStreamDeserializer(std::istream &);

		void Deserialize(ModelPartForRootParam) override;

	protected:
		std::istream & strm;
	};

	class DLL_PUBLIC XmlFileDeserializer : public Deserializer {
	public:
		explicit XmlFileDeserializer(std::filesystem::path);

		void Deserialize(ModelPartForRootParam) override;

	protected:
		const std::filesystem::path path;
	};

	class DLL_PUBLIC XmlDocumentDeserializer : public Deserializer {
	public:
		explicit XmlDocumentDeserializer(const xmlpp::Document *);

		void Deserialize(ModelPartForRootParam) override;

	protected:
		const xmlpp::Document * doc;
	};
}
