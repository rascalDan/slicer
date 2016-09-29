#ifndef SLICER_TEST_XML
#define SLICER_TEST_XML

#include <collections.ice>

module TestXml {
	struct BareContainers {
		[ "slicer:xml:bare" ]
		TestModule::Classes bareSeq;
		[ "slicer:xml:bare" ]
		TestModule::ClassMap bareMap;
	};
	struct EntityRef {
		[ "slicer:xml:attribute" ]
		int Id;
		[ "slicer:xml:text" ]
		string Name;
	};
};

#endif

