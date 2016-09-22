#ifndef SLICER_XML
#define SLICER_XML

#include <common.ice>

module Slicer {
	exception TooManyRowsReturned extends DeserializerError {};
	exception NoRowsReturned extends DeserializerError {};
	exception NoRowsFound extends SerializerError {};
	exception UnsuitableIdFieldType extends SerializerError {
		string type;
	};
};

#endif


