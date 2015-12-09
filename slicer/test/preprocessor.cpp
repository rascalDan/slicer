#define BOOST_TEST_MODULE preprocess
#include <boost/test/unit_test.hpp>

#include <slicer/parser.h>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <buffer.h>
#include <types.h>
#include "helpers.h"
#include "fileStructure.h"

namespace fs = boost::filesystem;

const unsigned int COMPONENTS_IN_TEST_ICE = 35;

BOOST_FIXTURE_TEST_SUITE ( preprocessor, FileStructure );

BOOST_AUTO_TEST_CASE( slicer_test_counts_path )
{
	auto count = Slicer::Slicer::Apply(slice, boost::filesystem::path("/dev/null"), {"-I" + included.string()});
	BOOST_REQUIRE_EQUAL(COMPONENTS_IN_TEST_ICE, count);
}

BOOST_AUTO_TEST_CASE( slicer_test_counts_filestar )
{
	FILE * file = fopen("/dev/null", "a");
	BOOST_REQUIRE(file);

	auto count = Slicer::Slicer::Apply(slice, file, {"-I" + included.string()});
	BOOST_REQUIRE_EQUAL(COMPONENTS_IN_TEST_ICE, count);

	fclose(file);
}

BOOST_AUTO_TEST_CASE( slicer_test_counts_nullfilestar )
{
	auto count = Slicer::Slicer::Apply(slice, NULL, {"-I" + included.string()});
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
	const std::string doslice = stringbf(
			"%s -I%s %s %s",
			root.parent_path() / "tool" / bjamout / "slicer",
			included,
			slice, cpp);
	BOOST_TEST_CHECKPOINT("slicer: " << doslice);
	system(doslice);

	const fs::path obj = fs::change_extension(tmp / base, ".o");
	const std::string compile = stringbf(
					"g++ -Os -fPIC -c -std=c++0x -fvisibility=hidden -I tmp -I /usr/include/adhocutil -I /usr/include/Ice -I /usr/include/IceUtil -I %s -I %s -I %s -I %s %s -o %s",
					root / bjamout,
					root,
					included / bjamout,
					root / "..",
					cpp, obj);
	BOOST_TEST_CHECKPOINT("compile: " << compile);
	system(compile);

	const fs::path so = fs::change_extension(tmp / ("libslicer" + slice.filename().string()), ".so");
	const std::string link = stringbf(
					"g++ -shared -lIce -lIceUtil %s/lib%s.so %s/lib%s.so %s -o %s",
					root / bjamout, base,
					included / bjamout, included.leaf(),
					obj, so);
	BOOST_TEST_CHECKPOINT("link: " << link);
	system(link);

	BOOST_TEST_CHECKPOINT("load: " << so);
	auto handle = loadlib(so);

	BOOST_TEST_CHECKPOINT("unload: " << handle);
	closelib(handle);
}

BOOST_AUTO_TEST_SUITE_END();

