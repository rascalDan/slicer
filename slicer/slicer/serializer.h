#ifndef SLICER_SERIALIZER_H
#define SLICER_SERIALIZER_H

#include <filesystem>
#include <slicer/modelParts.h>
#include <visibility.h>
#include <factory.h>

namespace Slicer {
	class DLL_PUBLIC Serializer {
		public:
			virtual ~Serializer() = default;

			virtual void Serialize(ModelPartForRootPtr) = 0;
	};
	typedef std::shared_ptr<Serializer> SerializerPtr;

	class DLL_PUBLIC Deserializer {
		public:
			virtual ~Deserializer() = default;

			virtual void Deserialize(ModelPartForRootPtr) = 0;
	};
	typedef std::shared_ptr<Deserializer> DeserializerPtr;

	typedef AdHoc::Factory<Serializer, std::ostream &> StreamSerializerFactory;
	typedef AdHoc::Factory<Deserializer, std::istream &> StreamDeserializerFactory;
	typedef AdHoc::Factory<Serializer, const std::filesystem::path &> FileSerializerFactory;
	typedef AdHoc::Factory<Deserializer, const std::filesystem::path &> FileDeserializerFactory;
}

#endif

