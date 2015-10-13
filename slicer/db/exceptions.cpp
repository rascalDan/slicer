#include "exceptions.h"

namespace Slicer {
	UnsupportedModelType::UnsupportedModelType() : std::invalid_argument("Unspported model type") { }
}

