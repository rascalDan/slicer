#ifndef SLICER_TEST_CLASSES
#define SLICER_TEST_CLASSES

#include <classtype.ice>
#include <structs.ice>

module TestModule {
	class DateTimeContainer {
		[	"slicer:conversion:boost.posix_time.ptime:ptimeToDateTime:dateTimeToPTime",
			"slicer:conversion:std.string:stringToDateTime:dateTimeToString:nodeclare" ]
		DateTime dt;
		IsoDate date;
	};
	class BuiltIns {
		bool mbool;
		byte mbyte;
		short mshort;
		["slicer:db:pkey",
		 "slicer:db:auto"]
		int mint;
		["slicer:db:pkey"]
		long mlong;
		float mfloat;
		double mdouble;
		string mstring;
	};
	class ClassClass {
		ClassType cls;
		StructType str;
		["slicer:xml:attribute"]
		int simp;
	};
};

#endif

