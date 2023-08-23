#ifndef SLICER_XML
#define SLICER_XML

#include <slicer/common.ice>

module Slicer {
	["cpp:ice_print"]
	exception BadBooleanValue extends DeserializerError {
		string text;
	};
	["cpp:ice_print"]
	exception BadNumericValue extends DeserializerError {
		string text;
	};
};

#endif

