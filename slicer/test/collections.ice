#ifndef SLICER_TEST_COLLECTIONS
#define SLICER_TEST_COLLECTIONS

#include <classes.ice>
#include <structs.ice>

module TestModule {
	sequence<string> SimpleSeq;
	sequence<BuiltIns> BuiltInSeq;
	sequence<ClassType> Classes;
	sequence<StructType> Structs;
	dictionary<int, ClassType> ClassMap;
	dictionary<int, StructType> StructMap;
};

#endif

