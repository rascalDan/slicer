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
		[	"slicer:conversion:boost.posix_time.ptime:boost.posix_time.to_iso_extended_string:boost.posix_time.time_from_string:nodeclare" ]
		optional(5) string optConverted;
	};
};

#endif

