#ifndef SLICER_TEST_COLLECTIONS
#define SLICER_TEST_COLLECTIONS

#include <classes.ice>
#include <structs.ice>

module TestModule {
	sequence<string> SimpleSeq;
	sequence<BuiltIns> BuiltInSeq;
	local sequence<StructType> Structs;
	dictionary<int, ClassType> ClassMap;
	local dictionary<int, StructType> StructMap;
	["slicer:value:res","slicer:key:id","slicer:item:thing"]
	local dictionary<long, StructType> StructMapNamed;
};

#endif

