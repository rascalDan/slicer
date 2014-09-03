#ifndef SLICER_SERIALIZER_H
#define SLICER_SERIALIZER_H

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <boost/filesystem/path.hpp>
#include <slicer/modelParts.h>

namespace Slicer {
	class Serializer : public IceUtil::Shared {
		public:
			virtual void Deserialize(ModelPartPtr) = 0;
			virtual void Serialize(ModelPartPtr) = 0;
	};
	typedef IceUtil::Handle<Serializer> SerializerPtr;
}

#endif

