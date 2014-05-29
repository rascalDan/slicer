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
		IceUtil::Handle<ModelPartForClassRoot<Object>> root = new ModelPartForClassRoot<Object>();
		SerializerPtr serializer = new Serializer();
		serializer->Deserialize(path, root);
		return &root->GetModel();
	}

	template <typename Serializer, typename Object>
	void
	Serialize(IceInternal::Handle<Object> object, const boost::filesystem::path & path)
	{
		IceUtil::Handle<ModelPartForClassRoot<Object>> root = new ModelPartForClassRoot<Object>(object);
		SerializerPtr serializer = new Serializer();
		serializer->Serialize(path, root);
	}
}

#endif

