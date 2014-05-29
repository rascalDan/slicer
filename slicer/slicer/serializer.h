#ifndef SLICER_SERIALIZER_H
#define SLICER_SERIALIZER_H

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <boost/filesystem/path.hpp>
#include <slicer/modelParts.h>

namespace Slicer {
	class Serializer : public IceUtil::Shared {
		public:
			virtual void Deserialize(const boost::filesystem::path &, ModelPartPtr) = 0;
			virtual void Serialize(const boost::filesystem::path &, ModelPartPtr) = 0;
	};
	typedef IceUtil::Handle<Serializer> SerializerPtr;
}

#endif

