#ifndef SLICER_H
#define SLICER_H

#include <Ice/Handle.h>
#include <slicer/modelParts.h>
#include <slicer/serializer.h>

namespace Slicer {
	template <typename Serializer, typename Object, typename ... SerializerParams>
	IceInternal::Handle<Object>
	Deserialize(SerializerParams & ... sp)
	{
		IceUtil::Handle<ModelPartForClassRoot<IceInternal::Handle<Object>>> root = new ModelPartForClassRoot<IceInternal::Handle<Object>>();
		SerializerPtr serializer = new Serializer(sp ...);
		serializer->Deserialize(root);
		return root->GetModel();
	}

	template <typename Serializer, typename Object, typename ... SerializerParams>
	void
	Serialize(IceInternal::Handle<Object> object, SerializerParams & ... sp)
	{
		IceUtil::Handle<ModelPartForClassRoot<IceInternal::Handle<Object>>> root = new ModelPartForClassRoot<IceInternal::Handle<Object>>(object);
		SerializerPtr serializer = new Serializer(sp ...);
		serializer->Serialize(root);
	}
}

#endif

