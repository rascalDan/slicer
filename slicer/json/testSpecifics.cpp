#define BOOST_TEST_MODULE json_specifics
#include <boost/test/unit_test.hpp>

#include "serializer.h"
#include <iostream>

BOOST_AUTO_TEST_CASE(factories)
{
	BOOST_REQUIRE(Slicer::FileSerializerFactory::createNew(".json", "/some.json"));
	BOOST_REQUIRE(Slicer::FileDeserializerFactory::createNew(".json", "/some.json"));
	BOOST_REQUIRE(Slicer::FileSerializerFactory::createNew(".js", "/some.js"));
	BOOST_REQUIRE(Slicer::FileDeserializerFactory::createNew(".js", "/some.js"));
	BOOST_REQUIRE(Slicer::StreamSerializerFactory::createNew("application/javascript", std::cout));
	BOOST_REQUIRE(Slicer::StreamDeserializerFactory::createNew("application/javascript", std::cin));
	BOOST_REQUIRE(Slicer::StreamSerializerFactory::createNew("application/javascript", std::cout));
	BOOST_REQUIRE(Slicer::StreamDeserializerFactory::createNew("application/javascript", std::cin));
}
