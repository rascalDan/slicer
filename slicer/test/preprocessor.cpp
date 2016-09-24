#define BOOST_TEST_MODULE preprocess
#include <boost/test/unit_test.hpp>

#include <tool/parser.h>
#include <boost/format.hpp>
#include <buffer.h>
#include "helpers.h"
#include "fileStructure.h"

namespace fs = boost::filesystem;

const unsigned int COMPONENTS_IN_TEST_ICE = 40;

BOOST_FIXTURE_TEST_SUITE ( preprocessor, FileStructure );

BOOST_AUTO_TEST_CASE( slicer_test_counts_path )
{
	Slicer::Slicer s;
	s.slicePath = slice;
	s.cppPath = "/dev/null";
	s.includes.push_back(included);

	auto count = s.Execute();
	BOOST_REQUIRE_EQUAL(COMPONENTS_IN_TEST_ICE, count);
	BOOST_REQUIRE_EQUAL(COMPONENTS_IN_TEST_ICE, s.Components());
}

BOOST_AUTO_TEST_CASE( slicer_test_counts_filestar )
{
	FILE * file = fopen("/dev/null", "a");
	BOOST_REQUIRE(file);
	Slicer::Slicer s;
	s.slicePath = slice;
	s.cpp = file;
	s.includes.push_back(included);

	auto count = s.Execute();
	BOOST_REQUIRE_EQUAL(COMPONENTS_IN_TEST_ICE, count);

	fclose(file);
}

BOOST_AUTO_TEST_CASE( slicer_test_counts_nullfilestar )
{
	Slicer::Slicer s;
	s.slicePath = slice;
	s.includes.push_back(included);

	auto count = s.Execute();
	BOOST_REQUIRE_EQUAL(COMPONENTS_IN_TEST_ICE, count);
}

BOOST_AUTO_TEST_CASE( slicer_test_counts_interfacesOnly )
{
	Slicer::Slicer s;
	s.slicePath = root / "interfaces.ice";

	auto count = s.Execute();
	BOOST_REQUIRE_EQUAL(0, count);
}

BOOST_AUTO_TEST_SUITE_END();

