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
	dictionary<string, string> StringMap;
	dictionary<string, EntityRef> RefMap;
	struct Maps {
		[ "slicer:xml:attributes" ]
		StringMap amap;
		[ "slicer:xml:elements" ]
		StringMap emap;
		[ "slicer:xml:elements" ]
		RefMap rmap;
	};
};

#endif

