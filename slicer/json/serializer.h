#ifndef SLICER_JSON_H
#define SLICER_JSON_H

#include <slicer/serializer.h>

namespace json {
	class Value;
}

namespace Slicer {
	class Json : public Serializer {
		public:
			Json(const boost::filesystem::path &);

			virtual void Deserialize(ModelPartPtr) override;
			virtual void Serialize(ModelPartPtr) override;

		protected:
			static void ModelTreeIterate(json::Value *, const std::string &, ModelPartPtr mp);
			static void ModelTreeIterateSeq(json::Value *, ModelPartPtr mp);
			static void ModelTreeIterateRoot(json::Value *, ModelPartPtr mp);

		private:
			const boost::filesystem::path path;
	};
}

#endif


