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
	class Optionals {
		optional(1) int simple;
		optional(2) StructType str;
	};
	sequence<ClassType> Classes;
	sequence<StructType> Structs;
	dictionary<int, ClassType> ClassMap;
	dictionary<int, StructType> StructMap;
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
};
