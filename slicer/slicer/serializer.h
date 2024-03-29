#pragma once

#include <c++11Helpers.h>
#include <factory.h> // IWYU pragma: keep
#include <filesystem>
#include <iosfwd>
#include <memory>
#include <slicer/modelParts.h>
#include <visibility.h>

// IWYU pragma: no_include "factory.impl.h"

namespace Slicer {
	class DLL_PUBLIC Serializer {
	public:
		Serializer() = default;
		SPECIAL_MEMBERS_DEFAULT(Serializer);

		virtual ~Serializer() = default;

		virtual void Serialize(ModelPartForRootParam) = 0;
	};

	using SerializerPtr = std::shared_ptr<Serializer>;

	class DLL_PUBLIC Deserializer {
	public:
		Deserializer() = default;
		SPECIAL_MEMBERS_DEFAULT(Deserializer);

		virtual ~Deserializer() = default;

		virtual void Deserialize(ModelPartForRootParam) = 0;
	};

	using DeserializerPtr = std::shared_ptr<Deserializer>;

	using StreamSerializerFactory = AdHoc::Factory<Serializer, std::ostream &>;
	using StreamDeserializerFactory = AdHoc::Factory<Deserializer, std::istream &>;
	using FileSerializerFactory = AdHoc::Factory<Serializer, const std::filesystem::path &>;
	using FileDeserializerFactory = AdHoc::Factory<Deserializer, const std::filesystem::path &>;
}
