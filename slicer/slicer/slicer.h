#ifndef SLICER_H
#define SLICER_H

#include <slicer/modelParts.h>
#include <slicer/serializer.h>

namespace Slicer {
	template <typename Object>
	Object
	DeserializeAnyWith(DeserializerPtr deserializer)
	{
		Object object;
		deserializer->Deserialize(ModelPart::CreateRootFor<Object>(object));
		return object;
	}

	template <typename Deserializer, typename Object, typename ... SerializerParams>
	Object
	DeserializeAny(SerializerParams && ... sp)
	{
		return DeserializeAnyWith<Object>(std::make_shared<Deserializer>(sp ...));
	}

	template <typename Object>
	void
	SerializeAnyWith(const Object & object, SerializerPtr serializer)
	{
		serializer->Serialize(ModelPart::CreateRootFor<const Object>(object));
	}

	template <typename Serializer, typename Object, typename ... SerializerParams>
	void
	SerializeAny(const Object & object, SerializerParams && ... sp)
	{
		SerializeAnyWith(object, std::make_shared<Serializer>(sp ...));
	}
}

#endif

