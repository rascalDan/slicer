#pragma once

#include <filesystem>
#include <fstream>
#include <iosfwd>
#include <jsonpp.h>
#include <slicer/modelParts.h>
#include <slicer/serializer.h>
#include <visibility.h>

namespace Slicer {
	class DLL_PUBLIC JsonValueSerializer : public Serializer {
	public:
		JsonValueSerializer() = default;
		~JsonValueSerializer() override;
		SPECIAL_MEMBERS_DEFAULT(JsonValueSerializer);

		void Serialize(ModelPartForRootParam) override;

	protected:
		json::Value value;
	};

	class DLL_PUBLIC JsonStreamSerializer : public JsonValueSerializer {
	public:
		explicit JsonStreamSerializer(std::ostream &);

		void Serialize(ModelPartForRootParam) override;

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

		void Deserialize(ModelPartForRootParam) override;

	protected:
		std::istream & strm;
	};

	class DLL_PUBLIC JsonFileDeserializer : public Deserializer {
	public:
		explicit JsonFileDeserializer(std::filesystem::path);

		void Deserialize(ModelPartForRootParam) override;

	protected:
		const std::filesystem::path path;
	};

	class DLL_PUBLIC JsonValueDeserializer : public Deserializer {
	public:
		explicit JsonValueDeserializer(const json::Value &);

		void Deserialize(ModelPartForRootParam) override;

	protected:
		const json::Value & value;
	};
}
