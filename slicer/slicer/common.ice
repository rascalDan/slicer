#ifndef SLICER
#define SLICER

module Slicer {
	exception CompilerError {
		string what;
	};
	exception RuntimeError { };
	exception SerializerError extends RuntimeError { };
	exception DeserializerError extends RuntimeError { };
	exception IncorrectElementName extends DeserializerError {
		string name;
	};
	exception UnsupportedModelType extends RuntimeError { };
	exception LocalTypeException extends RuntimeError { };
	exception NoConversionFound extends RuntimeError {
		string type;
	};
	exception UnknownType extends DeserializerError {
		string type;
	};
	exception InvalidEnumerationValue extends SerializerError {
		int value;
		string type;
	};
	exception InvalidEnumerationSymbol extends DeserializerError {
		string symbol;
		string type;
	};
};

#endif

