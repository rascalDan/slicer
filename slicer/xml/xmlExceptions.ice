#ifndef SLICER_XML
#define SLICER_XML

#include <common.ice>

module Slicer {
	["cpp:ice_print"]
	exception BadBooleanValue extends DeserializerError {
		string text;
	};
};

#endif

