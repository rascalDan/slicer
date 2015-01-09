#ifndef SLICER_H
#define SLICER_H

#include <Ice/Handle.h>
#include <slicer/modelParts.h>
#include <slicer/serializer.h>

namespace Slicer {
	template <typename Deserializer, typename Object, typename ... SerializerParams>
	Object
	DeserializeAny(SerializerParams & ... sp)
	{
		IceUtil::Handle<ModelPartForRoot<Object>> root = new ModelPartForRoot<Object>();
		DeserializerPtr deserializer = new Deserializer(sp ...);
		deserializer->Deserialize(root);
		return root->GetModel();
	}

	template <typename Deserializer, typename Object, typename ... SerializerParams>
	IceInternal::Handle<Object>
	Deserialize(SerializerParams & ... sp)
	{
		return DeserializeAny<Deserializer, IceInternal::Handle<Object>, SerializerParams...>(sp ...);
	}

	template <typename Serializer, typename Object, typename ... SerializerParams>
	void
	SerializeAny(Object object, SerializerParams & ... sp)
	{
		IceUtil::Handle<ModelPartForRoot<Object>> root = new ModelPartForRoot<Object>(object);
		SerializerPtr serializer = new Serializer(sp ...);
		serializer->Serialize(root);
	}

	template <typename Serializer, typename Object, typename ... SerializerParams>
	void
	Serialize(IceInternal::Handle<Object> object, SerializerParams & ... sp)
	{
		SerializeAny<Serializer>(object, sp ...);
	}
}

#endif

