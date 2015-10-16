#define BOOST_TEST_MODULE db_insert
#include <boost/test/unit_test.hpp>
#include <slicer/slicer.h>
#include "serializer.h"
#include <libxml++/parsers/domparser.h>
#include <types.h>

template <typename T, typename ... P>
T
BoostThrowWrapperHelper(P && ... p)
{
	return Slicer::DeserializeAny<Slicer::XmlDocumentDeserializer, T>(p...);
}

BOOST_AUTO_TEST_CASE( boolean_values )
{
	xmlpp::DomParser doc;
	doc.parse_memory("<Boolean>true</Boolean>");
	BOOST_REQUIRE_EQUAL(true, BoostThrowWrapperHelper<bool>(doc.get_document()));
	doc.parse_memory("<Boolean>false</Boolean>");
	BOOST_REQUIRE_EQUAL(false, BoostThrowWrapperHelper<bool>(doc.get_document()));
	doc.parse_memory("<Boolean>nonsense</Boolean>");
	BOOST_REQUIRE_THROW(BoostThrowWrapperHelper<bool>(doc.get_document()), Slicer::BadBooleanValue);
	doc.parse_memory("<Boolean> </Boolean>");
	BOOST_REQUIRE_THROW(BoostThrowWrapperHelper<bool>(doc.get_document()), Slicer::BadBooleanValue);
	doc.parse_memory("<Boolean></Boolean>");
	BOOST_REQUIRE_THROW(BoostThrowWrapperHelper<bool>(doc.get_document()), Slicer::BadBooleanValue);
}

BOOST_AUTO_TEST_CASE( int_values )
{
	xmlpp::DomParser doc;
	doc.parse_memory("<Int>13</Int>");
	BOOST_REQUIRE_EQUAL(13, BoostThrowWrapperHelper<Ice::Int>(doc.get_document()));
	doc.parse_memory("<Int>84</Int>");
	BOOST_REQUIRE_EQUAL(84, BoostThrowWrapperHelper<Ice::Int>(doc.get_document()));
	doc.parse_memory("<Int>0</Int>");
	BOOST_REQUIRE_EQUAL(0, BoostThrowWrapperHelper<Ice::Int>(doc.get_document()));
	doc.parse_memory("<Int>-4</Int>");
	BOOST_REQUIRE_EQUAL(-4, BoostThrowWrapperHelper<Ice::Int>(doc.get_document()));
	doc.parse_memory("<Int> </Int>");
	BOOST_REQUIRE_THROW(BoostThrowWrapperHelper<Ice::Int>(doc.get_document()), std::bad_cast);
	doc.parse_memory("<Int>notanint</Int>");
	BOOST_REQUIRE_THROW(BoostThrowWrapperHelper<Ice::Int>(doc.get_document()), std::bad_cast);
	doc.parse_memory("<Int></Int>");
	BOOST_REQUIRE_THROW(BoostThrowWrapperHelper<Ice::Int>(doc.get_document()), std::bad_cast);
}

