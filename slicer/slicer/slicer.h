#ifndef SLICER_H
#define SLICER_H

#include <memory>
#include <slicer/modelParts.h>
#include <slicer/serializer.h>

namespace Slicer {
	template<typename Object>
	Object
	DeserializeAnyWith(any_ptr<Deserializer> deserializer)
	{
		Object object {};
		ModelPart::OnRootFor<Object>(object, [deserializer](auto && mp) {
			deserializer->Deserialize(mp);
		});
		return object;
	}

	template<typename Deserializer, typename Object, typename... SerializerParams>
	Object
	DeserializeAny(SerializerParams &&... sp)
	{
		return DeserializeAnyWith<Object>(Deserializer(std::forward<SerializerParams>(sp)...));
	}

	template<typename Object>
	void
	SerializeAnyWith(const Object & object, any_ptr<Serializer> serializer)
	{
		ModelPart::OnRootFor<const Object>(object, [serializer](auto && mp) {
			serializer->Serialize(mp);
		});
	}

	template<typename Serializer, typename Object, typename... SerializerParams>
	void
	SerializeAny(const Object & object, SerializerParams &&... sp)
	{
		SerializeAnyWith(object, Serializer(std::forward<SerializerParams>(sp)...));
	}
}

#endif
