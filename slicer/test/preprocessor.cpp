#define BOOST_TEST_MODULE preprocess
#include <boost/test/unit_test.hpp>

#include <slicer/parser.h>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <misc.h>
#include <types.h>
#include "helpers.h"
#include "fileStructure.h"

namespace fs = boost::filesystem;

const unsigned int COMPONENTS_IN_TEST_ICE = 24;

BOOST_FIXTURE_TEST_SUITE ( preprocessor, FileStructure );

BOOST_AUTO_TEST_CASE( slicer_test_counts_path )
{
	auto count = Slicer::Slicer::Apply(slice, boost::filesystem::path("/dev/null"));
	BOOST_REQUIRE_EQUAL(COMPONENTS_IN_TEST_ICE, count);
}

BOOST_AUTO_TEST_CASE( slicer_test_counts_filestar )
{
	FILE * file = fopen("/dev/null", "a");
	BOOST_REQUIRE(file);

	auto count = Slicer::Slicer::Apply(slice, file);
	BOOST_REQUIRE_EQUAL(COMPONENTS_IN_TEST_ICE, count);

	fclose(file);
}

BOOST_AUTO_TEST_CASE( slicer_test_counts_nullfilestar )
{
	auto count = Slicer::Slicer::Apply(slice, NULL);
	BOOST_REQUIRE_EQUAL(COMPONENTS_IN_TEST_ICE, count);
}

BOOST_AUTO_TEST_CASE( slicer_test_counts_interfacesOnly )
{
	auto count = Slicer::Slicer::Apply(root / "interfaces.ice", NULL);
	BOOST_REQUIRE_EQUAL(0, count);
}

BOOST_AUTO_TEST_CASE( slicer_test_ice )
{
	const fs::path cpp = fs::change_extension(tmp / base, ".cpp");
	BOOST_TEST_CHECKPOINT("cpp: " << cpp);
	fs::remove(cpp);
	Slicer::Slicer::Apply(slice, cpp);

	const fs::path obj = fs::change_extension(tmp / base, ".o");
	const std::string compile = stringbf(
					"g++ -Os -fPIC -c -std=c++0x -I tmp -I /usr/include/Ice -I /usr/include/IceUtil -I %s -I %s %s -o %s",
					bjamout,
					root / "..",
					cpp, obj);
	BOOST_TEST_CHECKPOINT("compile: " << compile);
	system(compile);

	const fs::path so = fs::change_extension(tmp / ("libslicer" + slice.filename().string()), ".so");
	const std::string link = stringbf(
					"g++ -shared -lIce -lIceUtil %s/lib%s.so %s -o %s",
					bjamout, base,
					obj, so);
	BOOST_TEST_CHECKPOINT("link: " << link);
	system(link);

	BOOST_TEST_CHECKPOINT("load: " << so);
	auto handle = loadlib(so);

	BOOST_TEST_CHECKPOINT("unload: " << handle);
	closelib(handle);
}

BOOST_AUTO_TEST_SUITE_END();

