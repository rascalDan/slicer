#include "slicer.h"

namespace Slicer {
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
}

