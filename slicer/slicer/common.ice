#ifndef SLICER
#define SLICER

module Slicer {
	["cpp:ice_print"]
	exception CompilerError {
		string what;
	};
	["cpp:ice_print"]
	exception RuntimeError { };
	["cpp:ice_print"]
	exception SerializerError extends RuntimeError { };
	["cpp:ice_print"]
	exception DeserializerError extends RuntimeError { };
	["cpp:ice_print"]
	exception IncorrectElementName extends DeserializerError {
		string name;
	};
	["cpp:ice_print"]
	exception UnsupportedModelType extends RuntimeError { };
	["cpp:ice_print"]
	exception LocalTypeException extends RuntimeError {
		string type;
	};
	["cpp:ice_print"]
	exception NoConversionFound extends RuntimeError {
		string type;
	};
	["cpp:ice_print"]
	exception UnknownType extends DeserializerError {
		string type;
	};
	["cpp:ice_print"]
	exception IncorrectType extends DeserializerError {
		string type;
		string target;
	};
	["cpp:ice_print"]
	exception InvalidEnumerationValue extends SerializerError {
		int value;
		string type;
	};
	["cpp:ice_print"]
	exception InvalidEnumerationSymbol extends DeserializerError {
		string symbol;
		string type;
	};
	["cpp:ice_print"]
	exception InvalidStreamOperation extends SerializerError {
		string method;
	};
	["cpp:ice_print"]
	exception AbstractClassException extends DeserializerError {
		string type;
	};
};

#endif

