#define BOOST_TEST_MODULE preprocess
#include <boost/test/unit_test.hpp>

#include <tool/parser.h>
#include <common.h>
#include <boost/format.hpp>
#include <buffer.h>
#include <definedDirs.h>
#include "helpers.h"

namespace fs = boost::filesystem;

typedef std::map<std::string, unsigned int> ComponentsCount;
ComponentsCount COMPONENTS_IN_TEST_ICE = {
	{ "classes.ice", 4 },
	{ "collections.ice", 6 },
	{ "enums.ice", 2 },
	{ "inheritance.ice", 12 },
	{ "interfaces.ice", 0 },
	{ "json.ice", 2 },
	{ "locals.ice", 7 },
	{ "optionals.ice", 2 },
	{ "structs.ice", 4 },
	{ "types.ice", 3 },
	{ "xml.ice", 5 }
};

unsigned int
total()
{
	unsigned int t = 0;
	for(const auto & c : COMPONENTS_IN_TEST_ICE) {
		t += c.second;
	}
	return t;
	BOOST_REQUIRE_EQUAL(40, t);
}

void
process(Slicer::Slicer & s, const ComponentsCount::value_type & c)
{
#if BOOST_VERSION / 100 >= 1060
	BOOST_TEST_CONTEXT(c.first)
#endif
	{
		s.slicePath = rootDir / c.first;
		BOOST_REQUIRE_EQUAL(c.second, s.Execute());
	}
}

void
processAll(Slicer::Slicer & s)
{
	s.includes.push_back(rootDir / "included");
	s.includes.push_back(rootDir);
	for(const auto & c : COMPONENTS_IN_TEST_ICE) {
		BOOST_TEST_CHECKPOINT(c.first);
		process(s, c);
	}
	BOOST_REQUIRE_EQUAL(total(), s.Components());
}

BOOST_AUTO_TEST_CASE( slicer_test_counts_path )
{
	Slicer::Slicer s;
	s.cppPath = "/dev/null";
	processAll(s);
}

BOOST_AUTO_TEST_CASE( slicer_test_counts_filestar )
{
	FILE * file = fopen("/dev/null", "a");
	BOOST_REQUIRE(file);
	Slicer::Slicer s;
	s.cpp = file;
	processAll(s);
	fclose(file);
}

BOOST_AUTO_TEST_CASE( slicer_test_counts_nullfilestar )
{
	Slicer::Slicer s;
	processAll(s);
}

