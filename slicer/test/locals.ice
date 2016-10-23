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
};

#endif

