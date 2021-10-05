#ifndef SLICER_TEST_TYPES
#define SLICER_TEST_TYPES

[["cpp:include:boost/date_time/posix_time/posix_time.hpp"]]

#include <included.ice>
#include <enums.ice>
#include <structs.ice>
#include <classes.ice>
#include <collections.ice>
#include <optionals.ice>
#include <inheritance.ice>

["slicer:include:conversions.h"]
module TestModule {
	interface IgnoreMe {
		int someFunction();
		DontCountMe otherFileReference();
	};
	class CrossLibrary {
		DontCountMe otherFileReference;
	};
};

module TestModule2 {
	class CrossModule extends TestModule::ClassType {
		int anything;
		[	"slicer:conversion:boost.posix_time.ptime:ptimeToDateTime:dateTimeToPTime",
			"slicer:conversion:std.string:stringToDateTime:dateTimeToString:nodeclare" ]
		TestModule::DateTime dt;
		TestModule::Base base;
	};
	class Conv {
		[ "slicer:conversion:boost.posix_time.ptime:ptimeToString:stringToPtime:nodeclare" ]
		string conv;
	};
	class MissingConv {
		[ "slicer:conversion:boost.posix_time.ptime:ptimeToString:stringToPtime:nodeclare",
			"slicer:nodefaultconversion" ]
		string conv;
	};
};

#endif

