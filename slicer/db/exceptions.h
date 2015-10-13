#ifndef SLICER_DB_EXCEPTIONS_H
#define SLICER_DB_EXCEPTIONS_H

#include <stdexcept>

namespace Slicer {
	class UnsupportedModelType : public std::invalid_argument {
		public:
			UnsupportedModelType();
	};
}

#endif

