#ifndef SLICER_H
#define SLICER_H

#include <memory>
#include <slicer/modelParts.h>
#include <slicer/serializer.h>

namespace Slicer {
	template<typename Object>
	Object
	DeserializeAnyWith(const DeserializerPtr & deserializer)
	{
		Object object {};
		deserializer->Deserialize(ModelPart::CreateRootFor<Object>(object));
		return object;
	}

	template<typename Deserializer, typename Object, typename... SerializerParams>
	Object
	DeserializeAny(SerializerParams &&... sp)
	{
		return DeserializeAnyWith<Object>(std::make_shared<Deserializer>(std::forward<SerializerParams>(sp)...));
	}

	template<typename Object>
	void
	SerializeAnyWith(const Object & object, const SerializerPtr & serializer)
	{
		serializer->Serialize(ModelPart::CreateRootFor<const Object>(object));
	}

	template<typename Serializer, typename Object, typename... SerializerParams>
	void
	SerializeAny(const Object & object, SerializerParams &&... sp)
	{
		SerializeAnyWith(object, std::make_shared<Serializer>(std::forward<SerializerParams>(sp)...));
	}
}

#endif
