#include "slicer.h"

namespace Slicer {
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

