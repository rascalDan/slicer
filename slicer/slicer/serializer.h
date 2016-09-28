#ifndef SLICER_SERIALIZER_H
#define SLICER_SERIALIZER_H

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <boost/filesystem/path.hpp>
#include <slicer/modelParts.h>
#include <visibility.h>
#include <factory.h>

namespace Slicer {
	class DLL_PUBLIC Serializer : public IceUtil::Shared {
		public:
			virtual void Serialize(ModelPartForRootPtr) = 0;
	};
	typedef IceUtil::Handle<Serializer> SerializerPtr;

	class DLL_PUBLIC Deserializer : public IceUtil::Shared {
		public:
			virtual void Deserialize(ModelPartForRootPtr) = 0;
	};
	typedef IceUtil::Handle<Deserializer> DeserializerPtr;

	typedef AdHoc::Factory<Serializer, std::ostream &> StreamSerializerFactory;
	typedef AdHoc::Factory<Deserializer, std::istream &> StreamDeserializerFactory;
	typedef AdHoc::Factory<Serializer, const boost::filesystem::path &> FileSerializerFactory;
	typedef AdHoc::Factory<Deserializer, const boost::filesystem::path &> FileDeserializerFactory;
}

#endif

