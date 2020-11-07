#ifndef SLICER_TEST_CLASSTYPE
#define SLICER_TEST_CLASSTYPE

["slicer:include:conversions.h"]
module TestModule {
	[ "slicer:custommodelpart:TestModule.AbValidator" ]
	class ClassType {
		int a;
		int b;
	};
	["slicer:element:bis"]
	sequence<ClassType> Classes;
};

#endif

