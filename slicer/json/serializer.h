#ifndef SLICER_JSON_H
#define SLICER_JSON_H

#include <slicer/serializer.h>
#include <jsonpp.h>
#include <visibility.h>

namespace Slicer {
	class JsonSerializer : public Serializer {
		protected:
			static void ModelTreeIterate(json::Value *, const std::string &, ModelPartPtr mp);
			static void ModelTreeIterateDictObj(json::Value *, ModelPartPtr mp);
			static void ModelTreeIterateSeq(json::Value *, ModelPartPtr mp);
			static void ModelTreeIterateRoot(json::Value *, ModelPartPtr mp);
	};

	class DLL_PUBLIC JsonStreamSerializer : public JsonSerializer {
		public:
			JsonStreamSerializer(std::ostream &);

			virtual void Serialize(ModelPartForRootPtr) override;

		protected:
			std::ostream & strm;
	};

	class DLL_PUBLIC JsonFileSerializer : public JsonSerializer {
		public:
			JsonFileSerializer(std::filesystem::path);

			virtual void Serialize(ModelPartForRootPtr) override;

		protected:
			const std::filesystem::path path;
	};

	class DLL_PUBLIC JsonValueSerializer : public JsonSerializer {
		public:
			JsonValueSerializer(json::Value &);

			virtual void Serialize(ModelPartForRootPtr) override;

		protected:
			json::Value & value;
	};

	class DLL_PUBLIC JsonStreamDeserializer : public Deserializer {
		public:
			JsonStreamDeserializer(std::istream &);

			virtual void Deserialize(ModelPartForRootPtr) override;

		protected:
			std::istream & strm;
	};

	class DLL_PUBLIC JsonFileDeserializer : public Deserializer {
		public:
			JsonFileDeserializer(std::filesystem::path);

			virtual void Deserialize(ModelPartForRootPtr) override;

		protected:
			const std::filesystem::path path;
	};

	class DLL_PUBLIC JsonValueDeserializer : public Deserializer {
		public:
			JsonValueDeserializer(const json::Value &);

			virtual void Deserialize(ModelPartForRootPtr) override;

		protected:
			const json::Value & value;
	};
}

#endif


