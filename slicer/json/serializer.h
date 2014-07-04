#ifndef SLICER_JSON_H
#define SLICER_JSON_H

#include <slicer/serializer.h>

namespace json {
	class Value;
}

namespace Slicer {
	class Json : public Serializer {
		public:
			virtual void Deserialize(const boost::filesystem::path &, ModelPartPtr) override;
			virtual void Serialize(const boost::filesystem::path &, ModelPartPtr) override;

		protected:
			static void ModelTreeIterate(json::Value *, const std::string &, ModelPartPtr mp);
			static void ModelTreeIterateSeq(json::Value *, ModelPartPtr mp);
			static void ModelTreeIterateRoot(json::Value *, ModelPartPtr mp);
	};
}

#endif


