#ifndef SLICER_H
#define SLICER_H

#include <Ice/Handle.h>
#include <slicer/modelParts.h>
#include <slicer/serializer.h>

namespace Slicer {
	template <typename Deserializer, typename Object, typename ... SerializerParams>
	Object
	Deserialize(SerializerParams & ... sp)
	{
		IceUtil::Handle<ModelPartForRoot<Object>> root = new ModelPartForRoot<Object>();
		DeserializerPtr deserializer = new Deserializer(sp ...);
		deserializer->Deserialize(root);
		return root->GetModel();
	}

	template <typename Serializer, typename Object, typename ... SerializerParams>
	void
	Serialize(Object object, SerializerParams & ... sp)
	{
		IceUtil::Handle<ModelPartForRoot<Object>> root = new ModelPartForRoot<Object>(object);
		SerializerPtr serializer = new Serializer(sp ...);
		serializer->Serialize(root);
	}
}

#endif

