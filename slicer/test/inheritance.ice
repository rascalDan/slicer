#ifndef SLICER_TEST_INHERITANCE
#define SLICER_TEST_INHERITANCE

module TestModule {
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
};

#endif

