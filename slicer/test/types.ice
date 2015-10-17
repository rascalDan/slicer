[["cpp:include:boost/date_time/posix_time/posix_time_types.hpp"]]

#include <included.ice>

module TestModule {
	enum SomeNumbers {
		One = 1, Ten = 10, FiftyFive = 55
	};
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
		short month;
		short day;
	};
	class DateTimeContainer {
		[	"slicer:conversion:boost.posix_time.ptime:ptimeToDateTime:dateTimeToPTime",
			"slicer:conversion:std.string:stringToDateTime:dateTimeToString" ]
		DateTime dt;
		IsoDate date;
	};
	class BuiltIns {
		bool mbool;
		byte mbyte;
		short mshort;
		["slicer:db:pkey"]
		int mint;
		["slicer:db:pkey"]
		long mlong;
		float mfloat;
		double mdouble;
		string mstring;
	};
	class ClassType {
		int a;
		int b;
	};
	struct StructType {
		int a;
		int b;
	};
	sequence<BuiltIns> BuiltInSeq;
	sequence<ClassType> Classes;
	sequence<StructType> Structs;
	dictionary<int, ClassType> ClassMap;
	dictionary<int, StructType> StructMap;
	class Optionals {
		optional(0) int optSimple;
		optional(1) StructType optStruct;
		optional(2) ClassType optClass;
		optional(3) Classes optSeq;
		optional(4) ClassMap optDict;
	};
	class ClassClass {
		ClassType cls;
		StructType str;
		["slicer:xml:attribute"]
		int simp;
	};
	struct StructStruct {
		ClassType cls;
		StructType str;
		int simp;
	};
	class Base {
		int a;
	};
	class D1 extends Base {
		int b;
	};
	class D2 extends Base {
		int c;
	};
	class D3 extends D2 {
		int d;
	};
	sequence<Base> BaseSeq;
	dictionary<int, Base> BaseMap;
	class InheritanceCont {
		Base b;
		BaseSeq bs;
		BaseMap bm;
	};
	interface IgnoreMe {
		int someFunction();
		DontCountMe otherFileReference();
	};
	["slicer:typeid:mytype"]
	class Base2 {
		int a;
	};
	class D12 extends Base2 {
		int b;
	};
	class InheritanceCont2 {
		Base2 b;
	};
	["slicer:typename:onetwo"]
	class D13 extends Base2 {
		int c;
	};
	class InheritanceContMapped {
		Base2 b;
	};
	struct BareContainers {
		[ "slicer:xml:bare" ]
		Classes bareSeq;
		[ "slicer:xml:bare" ]
		ClassMap bareMap;
	};
	class SomeEnums {
		SomeNumbers one;
		SomeNumbers two;
	};
};

module TestModule2 {
	class CrossModule extends TestModule::ClassType {
		int anything;
		[	"slicer:conversion:boost.posix_time.ptime:ptimeToDateTime:dateTimeToPTime",
			"slicer:conversion:std.string:stringToDateTime:dateTimeToString" ]
		TestModule::DateTime dt;
		TestModule::Base base;
	};
	struct EntityRef {
		[ "slicer:xml:attribute" ]
		int Id;
		[ "slicer:xml:text" ]
		string Name;
	};
};

module DB {
	[	"slicer:conversion:boost.posix_time.time_duration:timedurationToTimespan:timespanToTimeduration" ]
	struct Timespan {
		int days;
		short hours;
		short minutes;
		short seconds;
	};
	class SpecificTypes extends TestModule::DateTimeContainer {
		Timespan ts;
	};
};

