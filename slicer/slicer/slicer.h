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
		Object object;
		deserializer->Deserialize(new ModelPartForRoot<Object>(object));
		return object;
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
		serializer->Serialize(new ModelPartForRoot<Object>(object));
	}

	template <typename Serializer, typename Object, typename ... SerializerParams>
	void
	SerializeAny(Object object, SerializerParams && ... sp)
	{
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

