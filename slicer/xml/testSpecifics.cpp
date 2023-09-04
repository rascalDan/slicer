#define BOOST_TEST_MODULE xml_specifics
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include "serializer.h"
#include <Ice/Config.h>
#include <iostream>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#ifndef __clang__
#	pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
#include <libxml++/parsers/domparser.h>
#pragma GCC diagnostic pop
#include <slicer/serializer.h>
#include <slicer/slicer.h>
#include <typeinfo>
#include <xmlExceptions.h>

// IWYU pragma: no_forward_declare Slicer::BadBooleanValue
// IWYU pragma: no_forward_declare Slicer::XmlDocumentDeserializer

template<typename out> using data = std::tuple<const char *, out>;
BOOST_FIXTURE_TEST_SUITE(doc, xmlpp::DomParser)

BOOST_DATA_TEST_CASE(good_boolean_values,
		boost::unit_test::data::make<data<bool>>({
				{"<Boolean>true</Boolean>", true},
				{"<Boolean>false</Boolean>", false},
		}),
		in, exp)
{
	parse_memory(in);
	BOOST_CHECK_EQUAL(exp, (Slicer::DeserializeAny<Slicer::XmlDocumentDeserializer, bool>(get_document())));
}

BOOST_DATA_TEST_CASE(bad_boolean_values,
		boost::unit_test::data::make({
				"<Boolean>nonsense</Boolean>",
				"<Boolean> </Boolean>",
				"<Boolean></Boolean>",
		}),
		in)
{
	parse_memory(in);
	BOOST_CHECK_THROW((std::ignore = Slicer::DeserializeAny<Slicer::XmlDocumentDeserializer, bool>(get_document())),
			Slicer::BadBooleanValue);
}

BOOST_DATA_TEST_CASE(good_integer_values,
		boost::unit_test::data::make<data<Ice::Int>>({
				{"<Int>13</Int>", 13},
				{"<Int>84</Int>", 84},
				{"<Int>0</Int>", 0},
				{"<Int>-4</Int>", -4},
		}),
		in, exp)
{
	parse_memory(in);
	BOOST_CHECK_EQUAL(exp, (Slicer::DeserializeAny<Slicer::XmlDocumentDeserializer, Ice::Int>(get_document())));
}

BOOST_DATA_TEST_CASE(bad_integer_values,
		boost::unit_test::data::make({
				"<Int> </Int>",
				"<Int>notanint</Int>",
				"<Int></Int>",
		}),
		in)
{
	parse_memory(in);
	BOOST_CHECK_THROW((std::ignore = Slicer::DeserializeAny<Slicer::XmlDocumentDeserializer, Ice::Int>(get_document())),
			Slicer::BadNumericValue);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(factories)
{
	BOOST_REQUIRE(Slicer::FileSerializerFactory::createNew(".xml", "/some.xml"));
	BOOST_REQUIRE(Slicer::FileDeserializerFactory::createNew(".xml", "/some.xml"));
	BOOST_REQUIRE(Slicer::StreamSerializerFactory::createNew("application/xml", std::cout));
	BOOST_REQUIRE(Slicer::StreamDeserializerFactory::createNew("application/xml", std::cin));
}
