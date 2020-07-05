#ifndef SLICER_TEST_STRUCTS
#define SLICER_TEST_STRUCTS

#include <classtype.ice>

["slicer:include:conversions.h"]
module TestModule {
	struct DateTime {
		short year;
		short month;
		short day;
		short hour;
		short minute;
		short second;
	};
	[ "slicer:conversion:std.string:stringToIsoDate:isoDateToString" ]
	struct IsoDate {
		short year;
		[ "slicer:custommodelpart:TestModule.MonthValidator" ]
		short month;
		short day;
	};
	struct StructType {
		int a;
		int b;
	};
	struct StructStruct {
		ClassType cls;
		StructType str;
		int simp;
	};
	sequence<IsoDate> Dates;
};

#endif

