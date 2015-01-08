#include "slicer.h"

namespace Slicer {
	const Metadata emptyMetadata;
#define MODELPARTFOR(Type, ModelPart) \
	ModelPartPtr ModelPartFor(Type & t) { return new ModelPart< Type >(t); } \
	ModelPartPtr ModelPartFor(Type * t) { return new ModelPart< Type >(t); }
	MODELPARTFOR(std::string, ModelPartForSimple);
	MODELPARTFOR(bool, ModelPartForSimple);
	MODELPARTFOR(Ice::Float, ModelPartForSimple);
	MODELPARTFOR(Ice::Double, ModelPartForSimple);
	MODELPARTFOR(Ice::Byte, ModelPartForSimple);
	MODELPARTFOR(Ice::Short, ModelPartForSimple);
	MODELPARTFOR(Ice::Int, ModelPartForSimple);
	MODELPARTFOR(Ice::Long, ModelPartForSimple);
#undef MODELPARTFOR

	Slicer::FunctionChildRef::FunctionChildRef(const Slicer::FunctionChildRef::ModelPartFunc & mp, const Slicer::FunctionChildRef::MetadataFunc & md) :
		mpf(mp),
		mdf(md)
	{
	}

	ModelPartPtr
	Slicer::FunctionChildRef::Child() const
	{
		return mpf();
	}

	const Metadata &
	Slicer::FunctionChildRef::ChildMetaData() const
	{
		return mdf();
	}

	Slicer::DirectChildRef::DirectChildRef(ModelPartPtr m) :
		mpp(m)
	{
	}

	ModelPartPtr
	Slicer::DirectChildRef::Child() const
	{
		return mpp;
	}

	const Metadata &
	Slicer::DirectChildRef::ChildMetaData() const
	{
		return emptyMetadata;
	}
}

