#ifndef SLICER_TEST_FUNCTIONS
#define SLICER_TEST_FUNCTIONS

["slicer:include:functionsImpl.h"]
module Functions {
	local class Funcs {
		void func();
	};
	["slicer:implementation:Functions.FuncsSubImpl"]
	local class FuncsSub extends Funcs {
		string testVal;
	};
	local struct SFuncs {
		Funcs obj;
	};
};

#endif

