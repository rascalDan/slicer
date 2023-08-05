#ifndef SLICER_JSON_H
#define SLICER_JSON_H

#include <filesystem>
#include <fstream>
#include <iosfwd>
#include <jsonpp.h>
#include <slicer/modelParts.h>
#include <slicer/serializer.h>
#include <string>
#include <visibility.h>

namespace Slicer {
	class JsonSerializer : public Serializer {
	protected:
		static void ModelTreeIterate(json::Value *, const std::string &, ModelPartParam mp);
		static void ModelTreeIterateDictObj(json::Value *, ModelPartParam mp);
		static void ModelTreeIterateSeq(json::Value *, ModelPartParam mp);
		static void ModelTreeIterateRoot(json::Value *, ModelPartParam mp);
	};

	class DLL_PUBLIC JsonValueSerializer : public JsonSerializer {
	public:
		void Serialize(ModelPartForRootPtr) override;

	protected:
		json::Value value;
	};

	class DLL_PUBLIC JsonStreamSerializer : public JsonValueSerializer {
	public:
		explicit JsonStreamSerializer(std::ostream &);

		void Serialize(ModelPartForRootPtr) override;

	protected:
		std::ostream & strm;
	};

	class DLL_PUBLIC JsonFileSerializer : public JsonStreamSerializer {
	public:
		explicit JsonFileSerializer(const std::filesystem::path &);

	protected:
		std::ofstream strm;
	};

	class DLL_PUBLIC JsonStreamDeserializer : public Deserializer {
	public:
		explicit JsonStreamDeserializer(std::istream &);

		void Deserialize(ModelPartForRootPtr) override;

	protected:
		std::istream & strm;
	};

	class DLL_PUBLIC JsonFileDeserializer : public Deserializer {
	public:
		explicit JsonFileDeserializer(std::filesystem::path);

		void Deserialize(ModelPartForRootPtr) override;

	protected:
		const std::filesystem::path path;
	};

	class DLL_PUBLIC JsonValueDeserializer : public Deserializer {
	public:
		explicit JsonValueDeserializer(const json::Value &);

		void Deserialize(ModelPartForRootPtr) override;

	protected:
		const json::Value & value;
	};
}

#endif
