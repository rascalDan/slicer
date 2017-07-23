#define BOOST_TEST_MODULE streams
#include <boost/test/unit_test.hpp>

#include <boost/lexical_cast.hpp>
#include <xml/serializer.h>
#include <json/serializer.h>
#include <slicer.h>
#include <collections.h>
#include <definedDirs.h>
#include "helpers.h"
#include "streams.h"

void TestStream::Produce(const Consumer & c)
{
	for (int x = 0; x < 10; x += 1) {
		auto str = boost::lexical_cast<std::string>(x);
		c(str);
	}
}

BOOST_FIXTURE_TEST_SUITE( stream, TestStream );

BOOST_AUTO_TEST_CASE( streamToXml )
{
	const auto outputXml = binDir / "streamOut.xml";
	Slicer::SerializeAny<Slicer::XmlFileSerializer, const TestStream>(*this, outputXml);
	diff(rootDir / "expected" / "streamOut.xml", outputXml);
	auto seq = Slicer::DeserializeAny<Slicer::XmlFileDeserializer, TestModule::SimpleSeq>(outputXml);
	BOOST_REQUIRE_EQUAL(10, seq.size());
	BOOST_REQUIRE_EQUAL("0", seq.front());
	BOOST_REQUIRE_EQUAL("9", seq.back());
}

BOOST_AUTO_TEST_CASE( streamToJson )
{
	const auto outputJson = binDir / "streamOut.json";
	Slicer::SerializeAny<Slicer::JsonFileSerializer, const TestStream>(*this, outputJson);
	diff(rootDir / "expected" / "streamOut.json", outputJson);
	auto seq = Slicer::DeserializeAny<Slicer::JsonFileDeserializer, TestModule::SimpleSeq>(outputJson);
	BOOST_REQUIRE_EQUAL(10, seq.size());
	BOOST_REQUIRE_EQUAL("0", seq.front());
	BOOST_REQUIRE_EQUAL("9", seq.back());
}

BOOST_AUTO_TEST_SUITE_END();

