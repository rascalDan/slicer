#ifndef SLICER_TEST_OPTIONALS
#define SLICER_TEST_OPTIONALS

#include <classes.ice>
#include <structs.ice>
#include <collections.ice>

module TestModule {
	class Optionals {
		optional(0) int optSimple;
		optional(1) StructType optStruct;
		optional(2) ClassType optClass;
		optional(3) Classes optSeq;
		optional(4) ClassMap optDict;
	};
};

#endif

