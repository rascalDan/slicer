#ifndef SLICER_TEST_LOCALS
#define SLICER_TEST_LOCALS

module Locals {
	local struct S {
		int a;
	};
	local sequence<S> Ss;
	local dictionary<int, S> Sd;
	local enum E {
		a, b
	};
	local class LocalClass {
		int a;
		string b;
	};
	local class LocalSubClass extends LocalClass {
		double c;
	};
	local class LocalSub2Class extends LocalSubClass {
		int d;
	};
};

#endif

