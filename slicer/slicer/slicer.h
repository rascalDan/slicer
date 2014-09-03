#ifndef SLICER_H
#define SLICER_H

#include <Ice/Handle.h>
#include <boost/filesystem/path.hpp>
#include <slicer/modelParts.h>
#include <slicer/serializer.h>

namespace Slicer {
	template <typename Serializer, typename Object>
	IceInternal::Handle<Object>
	Deserialize(const boost::filesystem::path & path)
	{
		IceUtil::Handle<ModelPartForClassRoot<IceInternal::Handle<Object>>> root = new ModelPartForClassRoot<IceInternal::Handle<Object>>();
		SerializerPtr serializer = new Serializer(path);
		serializer->Deserialize(root);
		return root->GetModel();
	}

	template <typename Serializer, typename Object>
	void
	Serialize(IceInternal::Handle<Object> object, const boost::filesystem::path & path)
	{
		IceUtil::Handle<ModelPartForClassRoot<IceInternal::Handle<Object>>> root = new ModelPartForClassRoot<IceInternal::Handle<Object>>(object);
		SerializerPtr serializer = new Serializer(path);
		serializer->Serialize(root);
	}
}

#endif

