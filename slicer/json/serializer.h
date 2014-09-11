#ifndef SLICER_JSON_H
#define SLICER_JSON_H

#include <slicer/serializer.h>
#include <jsonpp.h>

namespace Slicer {
	class Json : public Serializer {
		protected:
			static void ModelTreeIterate(json::Value *, const std::string &, ModelPartPtr mp);
			static void ModelTreeIterateSeq(json::Value *, ModelPartPtr mp);
			static void ModelTreeIterateRoot(json::Value *, ModelPartPtr mp);
	};

	class JsonFile : public Json {
		public:
			JsonFile(const boost::filesystem::path &);

			virtual void Deserialize(ModelPartPtr) override;
			virtual void Serialize(ModelPartPtr) override;

		private:
			const boost::filesystem::path path;
	};

	class JsonValue : public Json {
		public:
			JsonValue(json::Value &);

			virtual void Deserialize(ModelPartPtr) override;
			virtual void Serialize(ModelPartPtr) override;

		private:
			json::Value & value;
	};
}

#endif


