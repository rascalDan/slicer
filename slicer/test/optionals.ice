#ifndef SLICER_TEST_OPTIONALS
#define SLICER_TEST_OPTIONALS

#include <classes.ice>
#include <structs.ice>
#include <collections.ice>

module TestModule {
	class Optionals {
		optional(0) int optSimple;
		optional(1) StructType optStruct;
		optional(2) DateTimeContainer optClass;
		optional(3) Classes optSeq;
		optional(4) ClassMap optDict;
		[	"slicer:conversion:boost.posix_time.ptime:ptimeToString:stringToPtime:nodeclare" ]
		optional(5) string optConverted;
	};
	class Optionals2 {
		[	"slicer:conversion:std.string:Slicer.str2int:Slicer.int2str:nodeclare" ]
		optional(0) int optConverted;
		[	"slicer:conversion:std.string:Slicer.str2int:Slicer.int2str:nodeclare" ]
		int nonOptConverted;
	};
};

#endif

