#ifndef SLICER_TEST_FUNCITONSIMPL_H
#define SLICER_TEST_FUNCITONSIMPL_H

#include <functions.h>

namespace Functions {
	class FuncsSubImpl : public FuncsSub {
	public:
		void func() override;
	};
}

#endif
