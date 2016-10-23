#ifndef SLICER_XML
#define SLICER_XML

#include <common.ice>

module Slicer {
	["cpp:ice_print"]
	exception TooManyRowsReturned extends DeserializerError {};
	["cpp:ice_print"]
	exception NoRowsReturned extends DeserializerError {};
	["cpp:ice_print"]
	exception NoRowsFound extends SerializerError {};
	["cpp:ice_print"]
	exception UnsuitableIdFieldType extends SerializerError {
		string type;
	};
};

#endif


