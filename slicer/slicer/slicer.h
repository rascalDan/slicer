#ifndef SLICER_H
#define SLICER_H

#include <Ice/Handle.h>
#include <slicer/modelParts.h>
#include <slicer/serializer.h>

namespace Slicer {
	template <typename Deserializer, typename Object, typename ... SerializerParams>
	IceInternal::Handle<Object>
	Deserialize(SerializerParams && ... sp) __attribute__ ((deprecated));
	template <typename Deserializer, typename Object, typename ... SerializerParams>
	void
	Serialize(IceInternal::Handle<Object> object, SerializerParams && ... sp) __attribute__ ((deprecated));

	template <typename Object>
	Object
	DeserializeAnyWith(DeserializerPtr deserializer)
	{
		IceUtil::Handle<ModelPartForRoot<Object>> root = new ModelPartForRoot<Object>();
		deserializer->Deserialize(root);
		return root->GetModel();
	}

	template <typename Deserializer, typename Object, typename ... SerializerParams>
	Object
	DeserializeAny(SerializerParams && ... sp)
	{
		return DeserializeAnyWith<Object>(new Deserializer(sp ...));
	}

	template <typename Deserializer, typename Object, typename ... SerializerParams>
	IceInternal::Handle<Object>
	Deserialize(SerializerParams && ... sp)
	{
		return DeserializeAny<Deserializer, IceInternal::Handle<Object>, SerializerParams...>(sp ...);
	}

	template <typename Object>
	void
	SerializeAnyWith(Object object, SerializerPtr serializer)
	{
		IceUtil::Handle<ModelPartForRoot<Object>> root = new ModelPartForRoot<Object>(object);
		serializer->Serialize(root);
	}

	template <typename Serializer, typename Object, typename ... SerializerParams>
	void
	SerializeAny(Object object, SerializerParams && ... sp)
	{
		IceUtil::Handle<ModelPartForRoot<Object>> root = new ModelPartForRoot<Object>(object);
		SerializeAnyWith(object, new Serializer(sp ...));
	}

	template <typename Serializer, typename Object, typename ... SerializerParams>
	void
	Serialize(IceInternal::Handle<Object> object, SerializerParams && ... sp)
	{
		SerializeAny<Serializer>(object, sp ...);
	}
}

#endif

