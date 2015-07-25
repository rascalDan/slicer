#ifndef SLICER_JSON_H
#define SLICER_JSON_H

#include <slicer/serializer.h>
#include <jsonpp.h>

#ifndef DLL_PUBLIC
#define DLL_PUBLIC __attribute__ ((visibility ("default")))
#endif

namespace Slicer {
	class JsonSerializer : public Serializer {
		protected:
			static void ModelTreeIterate(json::Value *, const std::string &, ModelPartPtr mp);
			static void ModelTreeIterateSeq(json::Value *, ModelPartPtr mp);
			static void ModelTreeIterateRoot(json::Value *, ModelPartPtr mp);
	};

	class JsonFileSerializer : public JsonSerializer {
		public:
			DLL_PUBLIC JsonFileSerializer(const boost::filesystem::path &);

			virtual void Serialize(ModelPartPtr) override;

		private:
			const boost::filesystem::path path;
	};

	class JsonValueSerializer : public JsonSerializer {
		public:
			DLL_PUBLIC JsonValueSerializer(json::Value &);

			virtual void Serialize(ModelPartPtr) override;

		private:
			json::Value & value;
	};

	class JsonFileDeserializer : public Deserializer {
		public:
			DLL_PUBLIC JsonFileDeserializer(const boost::filesystem::path &);

			virtual void Deserialize(ModelPartPtr) override;

		private:
			const boost::filesystem::path path;
	};

	class JsonValueDeserializer : public Deserializer {
		public:
			DLL_PUBLIC JsonValueDeserializer(const json::Value &);

			virtual void Deserialize(ModelPartPtr) override;

		private:
			const json::Value & value;
	};
}

#endif


