#include "slicer.h"

namespace Slicer {
	const Metadata emptyMetadata;
#define MODELPARTFOR(Type, ModelPart) \
	ModelPartPtr ModelPartFor(IceUtil::Optional<Type> & t) { return new ModelPartForOptional< ModelPart<Type> >(t); } \
	ModelPartPtr ModelPartFor(IceUtil::Optional<Type> * t) { return new ModelPartForOptional< ModelPart<Type> >(t); } \
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

	Slicer::MemberChildRef::MemberChildRef(Slicer::ModelPartPtr mp, const Slicer::Metadata & md) :
		mpp(mp),
		mdr(md)
	{
	}

	ModelPartPtr
	Slicer::MemberChildRef::Child() const
	{
		return mpp;
	}

	const Metadata &
	Slicer::MemberChildRef::ChildMetaData() const
	{
		return mdr;
	}

	Slicer::ImplicitChildRef::ImplicitChildRef(ModelPartPtr m) :
		mpp(m)
	{
	}

	ModelPartPtr
	Slicer::ImplicitChildRef::Child() const
	{
		return mpp;
	}

	const Metadata &
	Slicer::ImplicitChildRef::ChildMetaData() const
	{
		return emptyMetadata;
	}
}

