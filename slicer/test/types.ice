module TestModule {
	class BuiltIns {
		bool mbool;
		byte mbyte;
		short mshort;
		int mint;
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
};
