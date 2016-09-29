#ifndef SLICER_TEST_ENUMS
#define SLICER_TEST_ENUMS

module TestModule {
	enum SomeNumbers {
		One = 1, Ten = 10, FiftyFive = 55
	};
	class SomeEnums {
		SomeNumbers one;
		SomeNumbers two;
	};
};

#endif

