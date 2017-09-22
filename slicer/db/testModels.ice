#ifndef SLICER_TEST_DB
#define SLICER_TEST_DB

#include <classes.ice>

module TestDatabase {
	[	"slicer:conversion:boost.posix_time.time_duration:timedurationToTimespan:timespanToTimeduration" ]
	class Timespan {
		int days;
		short hours;
		short minutes;
		short seconds;
	};
	class SpecificTypes extends TestModule::DateTimeContainer {
		Timespan ts;
	};
	class BuiltIns {
		optional(1) bool mbool;
		optional(2) byte mbyte;
		optional(3) short mshort;
		["slicer:db:pkey",
		 "slicer:db:auto"]
		int mint;
		["slicer:db:pkey"]
		long mlong;
		optional(4) float mfloat;
		optional(5) double mdouble;
		optional(6) string mstring;
	};
	sequence<BuiltIns> BuiltInSeq;
};

#endif

