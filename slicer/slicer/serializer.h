#ifndef SLICER_SERIALIZER_H
#define SLICER_SERIALIZER_H

#include <filesystem>
#include <slicer/modelParts.h>
#include <visibility.h>
#include <factory.h>

namespace Slicer {
	class DLL_PUBLIC Serializer {
		public:
			Serializer() = default;
			Serializer(const Serializer &) = delete;
			Serializer(Serializer &&) = delete;

			virtual ~Serializer() = default;

			Serializer & operator=(const Serializer &) = delete;
			Serializer & operator=(Serializer &&) = delete;

			virtual void Serialize(ModelPartForRootPtr) = 0;
	};
	using SerializerPtr = std::shared_ptr<Serializer>;

	class DLL_PUBLIC Deserializer {
		public:
			Deserializer() = default;
			Deserializer(const Deserializer &) = delete;
			Deserializer(Deserializer &&) = delete;

			virtual ~Deserializer() = default;

			Deserializer & operator=(const Deserializer &) = delete;
			Deserializer & operator=(Deserializer &&) = delete;

			virtual void Deserialize(ModelPartForRootPtr) = 0;
	};
	using DeserializerPtr = std::shared_ptr<Deserializer>;

	using StreamSerializerFactory = AdHoc::Factory<Serializer, std::ostream &>;
	using StreamDeserializerFactory = AdHoc::Factory<Deserializer, std::istream &>;
	using FileSerializerFactory = AdHoc::Factory<Serializer, const std::filesystem::path &>;
	using FileDeserializerFactory = AdHoc::Factory<Deserializer, const std::filesystem::path &>;
}

#endif

