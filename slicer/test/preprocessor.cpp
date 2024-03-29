#define BOOST_TEST_MODULE preprocess
#include <boost/test/unit_test.hpp>

#include "tool/icemetadata.h"
#include "tool/parser.h"
#include <cstdio>
#include <definedDirs.h>
#include <filesystem>
#include <numeric>
#include <span>
#include <utility>
#include <vector>

constexpr std::array<std::pair<std::string_view, size_t>, 12> COMPONENTS_IN_TEST_ICE_ARRAY {
		{{"classtype.ice", 2}, {"classes.ice", 3}, {"collections.ice", 6}, {"enums.ice", 2}, {"inheritance.ice", 12},
				{"interfaces.ice", 0}, {"json.ice", 2}, {"locals.ice", 7}, {"optionals.ice", 2}, {"structs.ice", 5},
				{"types.ice", 4}, {"xml.ice", 5}}};
using ComponentsCount = decltype(COMPONENTS_IN_TEST_ICE_ARRAY);
constexpr std::span<const ComponentsCount::value_type> COMPONENTS_IN_TEST_ICE {COMPONENTS_IN_TEST_ICE_ARRAY};
constexpr auto COMPONENTS_IN_TEST_ICE_COUNT = std::accumulate(
		COMPONENTS_IN_TEST_ICE.begin(), COMPONENTS_IN_TEST_ICE.end(), 0U, [](const auto & t, const auto & c) {
			return t + c.second;
		});
static_assert(COMPONENTS_IN_TEST_ICE_COUNT == 50);

void
process(Slicer::Slicer & s, const ComponentsCount::value_type & c)
{
	BOOST_TEST_CONTEXT(c.first) {
		s.slicePath = rootDir / c.first;
		// cppcheck-suppress assertWithSideEffect
		BOOST_REQUIRE_EQUAL(c.second, s.Execute());
	}
}

void
processAll(Slicer::Slicer & s)
{
	s.includes.push_back(rootDir / "included");
	s.includes.push_back(rootDir);
	for (const auto & c : COMPONENTS_IN_TEST_ICE) {
		BOOST_TEST_CONTEXT(c.first) {
			process(s, c);
		}
	}
	BOOST_REQUIRE_EQUAL(COMPONENTS_IN_TEST_ICE_COUNT, s.Components());
}

BOOST_AUTO_TEST_CASE(slicer_metadata_split_empty, *boost::unit_test::timeout(5))
{
	BOOST_CHECK(Slicer::IceMetaData::split("").empty());
}

BOOST_AUTO_TEST_CASE(slicer_metadata_split_single, *boost::unit_test::timeout(5))
{
	auto md {Slicer::IceMetaData::split("string")};
	BOOST_REQUIRE_EQUAL(md.size(), 1);
	BOOST_CHECK_EQUAL(md.front(), "string");
}

BOOST_AUTO_TEST_CASE(slicer_metadata_split_many, *boost::unit_test::timeout(5))
{
	auto md {Slicer::IceMetaData::split("string:and:some:other:values")};
	BOOST_REQUIRE_EQUAL(md.size(), 5);
	BOOST_CHECK_EQUAL(md.front(), "string");
	BOOST_CHECK_EQUAL(md.back(), "values");
}

BOOST_AUTO_TEST_CASE(slicer_test_counts_path, *boost::unit_test::timeout(5))
{
	Slicer::Slicer s;
	s.cppPath = "/dev/null";
	processAll(s);
}

BOOST_AUTO_TEST_CASE(slicer_test_counts_filestar, *boost::unit_test::timeout(5))
{
	FILE * file = fopen("/dev/null", "a");
	BOOST_REQUIRE(file);
	Slicer::Slicer s;
	s.cpp = file;
	processAll(s);
	fclose(file);
}

BOOST_AUTO_TEST_CASE(slicer_test_counts_nullfilestar, *boost::unit_test::timeout(5))
{
	Slicer::Slicer s;
	processAll(s);
}
