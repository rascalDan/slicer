#define BOOST_TEST_MODULE json_specifics
#include <boost/test/unit_test.hpp>
#include <slicer/slicer.h>
#include "serializer.h"
#include <types.h>

BOOST_AUTO_TEST_CASE( factories )
{
	BOOST_REQUIRE(Slicer::SerializerPtr(Slicer::FileSerializerFactory::createNew(".json", "/some.json")));
	BOOST_REQUIRE(Slicer::DeserializerPtr(Slicer::FileDeserializerFactory::createNew(".json", "/some.json")));
	BOOST_REQUIRE(Slicer::SerializerPtr(Slicer::FileSerializerFactory::createNew(".js", "/some.js")));
	BOOST_REQUIRE(Slicer::DeserializerPtr(Slicer::FileDeserializerFactory::createNew(".js", "/some.js")));
	BOOST_REQUIRE(Slicer::SerializerPtr(Slicer::StreamSerializerFactory::createNew("application/javascript", std::cout)));
	BOOST_REQUIRE(Slicer::DeserializerPtr(Slicer::StreamDeserializerFactory::createNew("application/javascript", std::cin)));
	BOOST_REQUIRE(Slicer::SerializerPtr(Slicer::StreamSerializerFactory::createNew("application/javascript", std::cout)));
	BOOST_REQUIRE(Slicer::DeserializerPtr(Slicer::StreamDeserializerFactory::createNew("application/javascript", std::cin)));
}

