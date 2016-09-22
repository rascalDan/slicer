#ifndef SLICER_XML
#define SLICER_XML

#include <common.ice>

module Slicer {
	exception BadBooleanValue extends DeserializerError {
		string text;
	};
};

#endif

