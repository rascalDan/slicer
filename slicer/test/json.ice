#ifndef SLICER_TEST_JSON
#define SLICER_TEST_JSON

module TestJson {
	[ "slicer:json:object" ]
	dictionary<string, int> Properties;
	class HasProperities {
		string name;
		Properties props;
	};
};

#endif

