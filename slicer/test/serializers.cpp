#define BOOST_TEST_MODULE execute_serializers
#include <boost/test/unit_test.hpp>

#include <tool/parser.h>
#include <common.h>
#include <slicer.h>
#include <modelParts.h>
#include <xml/serializer.h>
#include <libxml2/libxml/parser.h>
#include <json/serializer.h>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <types.h>
#include <json.h>
#include <xml.h>
#include <fstream>
#include "helpers.h"
#include <definedDirs.h>
#include "conversions.h"

namespace fs = boost::filesystem;

// LCOV_EXCL_START
BOOST_TEST_DONT_PRINT_LOG_VALUE ( TestModule::ClassMap::iterator )
// LCOV_EXCL_STOP

class FileBased {
	public:
		template<typename T, typename DeserializerIn>
		void
		verifyByFile(const fs::path & infile, const boost::function<void(const T &)> & check = NULL)
		{
			const fs::path input = rootDir / "initial" / infile;
			const fs::path tmpf = binDir / "byFile";
			fs::create_directory(tmpf);
			const fs::path output = tmpf / infile;
			const fs::path outputJson = tmpf / fs::change_extension(infile, "json");
			const fs::path outputXml = tmpf / fs::change_extension(infile, "xml");

			BOOST_TEST_CHECKPOINT("Deserialize: " << input);
			T p = Slicer::DeserializeAny<DeserializerIn, T>(input);

			if (check) {
				BOOST_TEST_CHECKPOINT("Check1: " << input);
				check(p);
			}

			BOOST_TEST_CHECKPOINT("Serialize " << input << " -> " << outputJson);
			Slicer::SerializeAny<Slicer::JsonFileSerializer>(p, outputJson);

			BOOST_TEST_CHECKPOINT("Serialize " << input << " -> " << outputXml);
			Slicer::SerializeAny<Slicer::XmlFileSerializer>(p, outputXml);

			if (check) {
				BOOST_TEST_CHECKPOINT("Check2: " << input);
				check(p);
			}

			BOOST_TEST_CHECKPOINT("Checksum: " << input << " === " << output);
			diff(input, output);
		}

		template<typename T, typename Deserializer, typename Serializer, typename Internal>
		void
		verifyByHelper(const fs::path & infile,
				const boost::function<Internal(const fs::path &)> & in,
				const boost::function<void(const Internal &, const fs::path &)> & out,
				const boost::function<void(Internal &)> & ifree,
				const boost::function<void(const T &)> & check = NULL)
		{
			const fs::path input = rootDir / "initial" / infile;
			const fs::path tmph = binDir / "byHandler";
			fs::create_directory(tmph);
			const fs::path output = tmph / infile;

			BOOST_TEST_CHECKPOINT("Read: " << input);
			Internal docRead = in(input);

			BOOST_TEST_CHECKPOINT("Deserialize: " << input);
			T p = Slicer::DeserializeAny<Deserializer, T>(docRead);
			ifree(docRead);

			if (check) {
				BOOST_TEST_CHECKPOINT("Check1: " << input);
				check(p);
			}

			BOOST_TEST_CHECKPOINT("Serialize: " << input);
			Internal docWrite;
			Slicer::SerializeAny<Serializer>(p, docWrite);

			if (check) {
				BOOST_TEST_CHECKPOINT("Check2: " << input);
				check(p);
			}

			BOOST_TEST_CHECKPOINT("Write: " << output);
			out(docWrite, output);
			ifree(docWrite);

			BOOST_TEST_CHECKPOINT("Checksum: " << input << " === " << output);
			diff(input, output);
		}
};

void
checkBuiltIns_valuesCorrect(const TestModule::BuiltInsPtr & bt)
{
	BOOST_REQUIRE_EQUAL(bt->mbool, true);
	BOOST_REQUIRE_EQUAL(bt->mbyte, 4);
	BOOST_REQUIRE_EQUAL(bt->mshort, 40);
	BOOST_REQUIRE_EQUAL(bt->mint, 80);
	BOOST_REQUIRE_EQUAL(bt->mlong, 800);
	BOOST_REQUIRE_EQUAL(bt->mfloat, 3.125);
	BOOST_REQUIRE_EQUAL(bt->mdouble, 3.0625);
	BOOST_REQUIRE_EQUAL(bt->mstring, "Sample text");
}

void
checkBuiltIns3_valuesCorrect(const TestModule::BuiltInsPtr & bt)
{
	BOOST_REQUIRE_EQUAL(bt->mbool, false);
	BOOST_REQUIRE_EQUAL(bt->mbyte, 255);
	BOOST_REQUIRE_EQUAL(bt->mshort, -40);
	BOOST_REQUIRE_EQUAL(bt->mint, -80);
	BOOST_REQUIRE_EQUAL(bt->mlong, -800);
	BOOST_REQUIRE_EQUAL(bt->mfloat, -3.125);
	BOOST_REQUIRE_EQUAL(bt->mdouble, -3.0625);
	BOOST_REQUIRE_EQUAL(bt->mstring, "-Sample text-");
}

void
checkInherits_types(const TestModule::InheritanceContPtr & i)
{
	BOOST_REQUIRE(i->b);
	BOOST_REQUIRE(TestModule::D1Ptr::dynamicCast(i->b));
	BOOST_REQUIRE_EQUAL(TestModule::D1Ptr::dynamicCast(i->b)->a, 1);
	BOOST_REQUIRE_EQUAL(TestModule::D1Ptr::dynamicCast(i->b)->b, 2);
	BOOST_REQUIRE_EQUAL(i->bs.size(), 3);
	BOOST_REQUIRE(i->bs[0]);
	BOOST_REQUIRE(TestModule::D2Ptr::dynamicCast(i->bs[0]));
	BOOST_REQUIRE_EQUAL(TestModule::D2Ptr::dynamicCast(i->bs[0])->a, 1);
	BOOST_REQUIRE_EQUAL(TestModule::D2Ptr::dynamicCast(i->bs[0])->c, 100);
	BOOST_REQUIRE(i->bs[1]);
	BOOST_REQUIRE(TestModule::D3Ptr::dynamicCast(i->bs[1]));
	BOOST_REQUIRE_EQUAL(TestModule::D3Ptr::dynamicCast(i->bs[1])->a, 2);
	BOOST_REQUIRE_EQUAL(TestModule::D3Ptr::dynamicCast(i->bs[1])->c, 100);
	BOOST_REQUIRE_EQUAL(TestModule::D3Ptr::dynamicCast(i->bs[1])->d, 200);
	BOOST_REQUIRE(i->bs[2]);
	BOOST_REQUIRE_EQUAL(i->bs[2]->a, 3);
	BOOST_REQUIRE(!TestModule::D1Ptr::dynamicCast(i->bs[2]));
	BOOST_REQUIRE(!TestModule::D2Ptr::dynamicCast(i->bs[2]));
	BOOST_REQUIRE(!TestModule::D3Ptr::dynamicCast(i->bs[2]));
	BOOST_REQUIRE_EQUAL(i->bm.size(), 3);
	BOOST_REQUIRE(TestModule::D1Ptr::dynamicCast(i->bm.find(10)->second));
	BOOST_REQUIRE(TestModule::D3Ptr::dynamicCast(i->bm.find(12)->second));
	BOOST_REQUIRE(!TestModule::D1Ptr::dynamicCast(i->bm.find(14)->second));
	BOOST_REQUIRE(!TestModule::D2Ptr::dynamicCast(i->bm.find(14)->second));
	BOOST_REQUIRE(!TestModule::D3Ptr::dynamicCast(i->bm.find(14)->second));
}

void
checkOptionals_notset(const TestModule::OptionalsPtr & opts)
{
	BOOST_REQUIRE(!opts->optSimple);
	BOOST_REQUIRE(!opts->optStruct);
	BOOST_REQUIRE(!opts->optClass);
	BOOST_REQUIRE(!opts->optSeq);
	BOOST_REQUIRE(!opts->optDict);
}

void
checkOptionals_areset(const TestModule::OptionalsPtr & opts)
{
	BOOST_REQUIRE(opts->optSimple);
	BOOST_REQUIRE_EQUAL(opts->optSimple, 4);
	BOOST_REQUIRE(opts->optStruct);
	BOOST_REQUIRE_EQUAL(opts->optStruct->a, 1);
	BOOST_REQUIRE_EQUAL(opts->optStruct->b, 2);
	BOOST_REQUIRE(opts->optClass);
	BOOST_REQUIRE_EQUAL((*opts->optClass)->a, 1);
	BOOST_REQUIRE_EQUAL((*opts->optClass)->b, 2);
	BOOST_REQUIRE_EQUAL(opts->optSeq->size(), 2);
	BOOST_REQUIRE_EQUAL((*opts->optSeq)[0]->a, 3);
	BOOST_REQUIRE_EQUAL((*opts->optSeq)[0]->b, 4);
	BOOST_REQUIRE_EQUAL((*opts->optSeq)[1]->a, 5);
	BOOST_REQUIRE_EQUAL((*opts->optSeq)[1]->b, 6);
	BOOST_REQUIRE(opts->optDict);
	BOOST_REQUIRE_EQUAL(opts->optDict->size(), 2);
	BOOST_REQUIRE_EQUAL(opts->optDict->find(1), opts->optDict->end());
	BOOST_REQUIRE(opts->optDict->find(10) != opts->optDict->end());
	BOOST_REQUIRE_EQUAL(opts->optDict->find(10)->second->a, 11);
	BOOST_REQUIRE_EQUAL(opts->optDict->find(10)->second->b, 12);
	BOOST_REQUIRE(opts->optDict->find(13) != opts->optDict->end());
	BOOST_REQUIRE_EQUAL(opts->optDict->find(13)->second->a, 14);
	BOOST_REQUIRE_EQUAL(opts->optDict->find(13)->second->b, 15);
}

void
checkSeqOfClass(const TestModule::Classes & seqOfClass)
{
	BOOST_REQUIRE_EQUAL(seqOfClass.size(), 2);
	BOOST_REQUIRE_EQUAL(seqOfClass[0]->a, 1);
	BOOST_REQUIRE_EQUAL(seqOfClass[0]->b, 2);
	BOOST_REQUIRE_EQUAL(seqOfClass[1]->a, 4);
	BOOST_REQUIRE_EQUAL(seqOfClass[1]->b, 5);
}

void
checkStruct(const TestModule::StructType & st)
{
	BOOST_REQUIRE_EQUAL(st.a, 10);
	BOOST_REQUIRE_EQUAL(st.b, 13);
}

template<class T>
void
checkAssertEq(const T & expected, const T & actual)
{
	BOOST_REQUIRE_EQUAL(expected, actual);
}

void
checkEntityRef(const TestXml::EntityRef & er)
{
	BOOST_REQUIRE_EQUAL(er.Id, 26);
	BOOST_REQUIRE_EQUAL(er.Name, "Hull City");
}

void
checkBare(const TestXml::BareContainers & bc)
{
	BOOST_REQUIRE_EQUAL(bc.bareSeq.size(), 2);
	BOOST_REQUIRE_EQUAL(bc.bareSeq[0]->a, 1);
	BOOST_REQUIRE_EQUAL(bc.bareSeq[0]->b, 2);
	BOOST_REQUIRE_EQUAL(bc.bareSeq[1]->a, 3);
	BOOST_REQUIRE_EQUAL(bc.bareSeq[1]->b, 4);
	BOOST_REQUIRE_EQUAL(bc.bareMap.size(), 2);
}

void
checkSomeEnums(const TestModule::SomeEnumsPtr & se)
{
	BOOST_REQUIRE_EQUAL(se->one, TestModule::Ten);
	BOOST_REQUIRE_EQUAL(se->two, TestModule::FiftyFive);
}

void
checkSomeNumbers(const TestModule::SomeNumbers & sn)
{
	BOOST_REQUIRE_EQUAL(sn, TestModule::FiftyFive);
}

void
checkObjectMap(const TestJson::Properties & p)
{
	BOOST_REQUIRE_EQUAL(3, p.size());
	BOOST_REQUIRE_EQUAL(1, p.find("one")->second);
	BOOST_REQUIRE_EQUAL(20, p.find("twenty")->second);
	BOOST_REQUIRE_EQUAL(300, p.find("three hundred")->second);
}

void
checkObjectMapMember(const TestJson::HasProperitiesPtr & p)
{
	BOOST_REQUIRE_EQUAL(p->name, "foo");
	checkObjectMap(p->props);
}

xmlpp::Document *
readXml(const fs::path & path)
{
	return new xmlpp::Document(xmlParseFile(path.string().c_str()));
}

void
writeXml(xmlpp::Document * const & doc, const fs::path & path)
{
	doc->write_to_file_formatted(path.string());
}

void
freeXml(xmlpp::Document * & doc)
{
	delete doc;
}

json::Value
readJson(const fs::path & path)
{
	std::ifstream inFile(path.string());
	std::stringstream buffer;
	buffer << inFile.rdbuf();
	Glib::ustring doc(buffer.str());
	Glib::ustring::const_iterator itr = doc.begin();
	return json::parseValue(itr);
}

void
writeJson(const json::Value & value, const fs::path & path)
{
	std::ofstream outFile(path.string());
	json::serializeValue(value, outFile, "utf-8");
}

void
freeJson(json::Value &)
{
}

BOOST_FIXTURE_TEST_SUITE ( byFile, FileBased );

BOOST_AUTO_TEST_CASE( builtins_xml )
{
	verifyByFile<TestModule::BuiltInsPtr, Slicer::XmlFileDeserializer>("builtins.xml", checkBuiltIns_valuesCorrect);
}

BOOST_AUTO_TEST_CASE( structtype_xml )
{
	verifyByFile<TestModule::StructType, Slicer::XmlFileDeserializer>("struct.xml", checkStruct);
}

BOOST_AUTO_TEST_CASE( structtype_json )
{
	verifyByFile<TestModule::StructType, Slicer::JsonFileDeserializer>("struct2.json", checkStruct);
}

BOOST_AUTO_TEST_CASE( simplestring_xml )
{
	verifyByFile<std::string, Slicer::XmlFileDeserializer>("string.xml", boost::bind(checkAssertEq<std::string>, "test string", _1));
}

BOOST_AUTO_TEST_CASE( simpleint_xml )
{
	verifyByFile<Ice::Int, Slicer::XmlFileDeserializer>("int.xml", boost::bind(checkAssertEq<Ice::Int>, 27, _1));
}

BOOST_AUTO_TEST_CASE( simplestring_json )
{
	verifyByFile<std::string, Slicer::JsonFileDeserializer>("string2.json", boost::bind(checkAssertEq<std::string>, "test string", _1));
}

BOOST_AUTO_TEST_CASE( simpleint_json )
{
	verifyByFile<Ice::Int, Slicer::JsonFileDeserializer>("int2.json", boost::bind(checkAssertEq<Ice::Int>, 27, _1));
}

BOOST_AUTO_TEST_CASE( complexClass_xmlattrAndText )
{
	verifyByFile<TestXml::EntityRef, Slicer::XmlFileDeserializer>("entityref.xml", checkEntityRef);
}

BOOST_AUTO_TEST_CASE( sequenceOfClass_xml )
{
	verifyByFile<TestModule::Classes, Slicer::XmlFileDeserializer>("seqOfClass.xml", checkSeqOfClass);
}

BOOST_AUTO_TEST_CASE( sequenceOfClass_json )
{
	verifyByFile<TestModule::Classes, Slicer::JsonFileDeserializer>("seqOfClass2.json", checkSeqOfClass);
}

BOOST_AUTO_TEST_CASE( optionals_notset_xml )
{
	verifyByFile<TestModule::OptionalsPtr, Slicer::XmlFileDeserializer>("optionals-notset.xml", checkOptionals_notset);
}

BOOST_AUTO_TEST_CASE( optionals_areset_xml )
{
	verifyByFile<TestModule::OptionalsPtr, Slicer::XmlFileDeserializer>("optionals-areset.xml", checkOptionals_areset);
}

BOOST_AUTO_TEST_CASE( inherit_a_xml )
{
	verifyByFile<TestModule::InheritanceContPtr, Slicer::XmlFileDeserializer>("inherit-a.xml");
}

BOOST_AUTO_TEST_CASE( inherit_b_xml )
{
	verifyByFile<TestModule::InheritanceContPtr, Slicer::XmlFileDeserializer>("inherit-b.xml", checkInherits_types);
}

BOOST_AUTO_TEST_CASE( conv_datetime_xml )
{
	verifyByFile<TestModule::DateTimeContainerPtr, Slicer::XmlFileDeserializer>("conv-datetime.xml");
}

BOOST_AUTO_TEST_CASE( builtins2_json )
{
	verifyByFile<TestModule::BuiltInsPtr, Slicer::JsonFileDeserializer>("builtins2.json", checkBuiltIns_valuesCorrect);
}

BOOST_AUTO_TEST_CASE( builtins3_json )
{
	verifyByFile<TestModule::BuiltInsPtr, Slicer::JsonFileDeserializer>("builtins3.json", checkBuiltIns3_valuesCorrect);
}

BOOST_AUTO_TEST_CASE( optionals_areset2_json )
{
	verifyByFile<TestModule::OptionalsPtr, Slicer::JsonFileDeserializer>("optionals-areset2.json", checkOptionals_areset);
}

BOOST_AUTO_TEST_CASE( inherit_c_json )
{
	verifyByFile<TestModule::InheritanceContPtr, Slicer::JsonFileDeserializer>("inherit-c.json", checkInherits_types);
}

BOOST_AUTO_TEST_CASE( inherit_d_json )
{
	verifyByFile<TestModule::InheritanceCont2Ptr, Slicer::JsonFileDeserializer>("inherit-d.json");
}

BOOST_AUTO_TEST_CASE( inherit_mapped_json )
{
	verifyByFile<TestModule::InheritanceContMappedPtr, Slicer::JsonFileDeserializer>("inherit-mapped.json");
}

BOOST_AUTO_TEST_CASE( xml_attribute_xml )
{
	verifyByFile<TestModule::ClassClassPtr, Slicer::XmlFileDeserializer>("xmlattr.xml");
}

BOOST_AUTO_TEST_CASE( xml_barecontainers_xml )
{
	verifyByFile<TestXml::BareContainers, Slicer::XmlFileDeserializer>("bare.xml", checkBare);
}

BOOST_AUTO_TEST_CASE( xml_classOfEnums_xml )
{
	verifyByFile<TestModule::SomeEnumsPtr, Slicer::XmlFileDeserializer>("someenums.xml", checkSomeEnums);
}

BOOST_AUTO_TEST_CASE( xml_rootEnums_xml )
{
	verifyByFile<TestModule::SomeNumbers, Slicer::XmlFileDeserializer>("enum.xml", checkSomeNumbers);
}

BOOST_AUTO_TEST_CASE( json_rootEnums_json )
{
	verifyByFile<TestModule::SomeNumbers, Slicer::JsonFileDeserializer>("enum2.json", checkSomeNumbers);
}

BOOST_AUTO_TEST_CASE( json_objectmap )
{
	verifyByFile<TestJson::Properties, Slicer::JsonFileDeserializer>("objectmap.json", checkObjectMap);
}

BOOST_AUTO_TEST_CASE( json_objectmapMember )
{
	verifyByFile<TestJson::HasProperitiesPtr, Slicer::JsonFileDeserializer>("objectmapMember.json", checkObjectMapMember);
}

BOOST_AUTO_TEST_CASE( json_simpleArray )
{
	verifyByFile<TestModule::SimpleSeq, Slicer::JsonFileDeserializer>("simpleArray1.json");
}

BOOST_AUTO_TEST_CASE( xml_simpleArray )
{
	verifyByFile<TestModule::SimpleSeq, Slicer::XmlFileDeserializer>("simpleArray2.xml");
}

BOOST_AUTO_TEST_CASE( json_streams )
{
	const auto tmpf = binDir / "byStream";
	const auto inFile = rootDir / "initial" / "inherit-c.json";
	const auto outFile = tmpf / "streamout.json";
	boost::filesystem::create_directories(tmpf);
	{
		std::ifstream in(inFile.string());
		auto d = Slicer::DeserializeAny<Slicer::JsonStreamDeserializer, TestModule::InheritanceContPtr>(in);
		checkInherits_types(d);
		std::ofstream out(outFile.string());
		Slicer::SerializeAny<Slicer::JsonStreamSerializer>(d, out);
	}
	diff(inFile, outFile);
}

BOOST_AUTO_TEST_CASE( xml_streams )
{
	const auto tmpf = binDir / "byStream";
	const auto inFile = rootDir / "initial" / "inherit-b.xml";
	const auto outFile = tmpf / "streamout.xml";
	boost::filesystem::create_directories(tmpf);
	{
		std::ifstream in(inFile.string());
		auto d = Slicer::DeserializeAny<Slicer::XmlStreamDeserializer, TestModule::InheritanceContPtr>(in);
		checkInherits_types(d);
		std::ofstream out(outFile.string());
		Slicer::SerializeAny<Slicer::XmlStreamSerializer>(d, out);
	}
	diff(inFile, outFile);
}

BOOST_AUTO_TEST_CASE( invalid_enum )
{
	Slicer::DeserializerPtr jdeserializer = new Slicer::JsonFileDeserializer(rootDir / "initial" / "invalidEnum.json");
	BOOST_REQUIRE_THROW(Slicer::DeserializeAnyWith<TestModule::SomeNumbers>(jdeserializer), Slicer::InvalidEnumerationSymbol);

	Slicer::DeserializerPtr xdeserializer = new Slicer::XmlFileDeserializer(rootDir / "initial" / "invalidEnum.xml");
	BOOST_REQUIRE_THROW(Slicer::DeserializeAnyWith<TestModule::SomeNumbers>(xdeserializer), Slicer::InvalidEnumerationSymbol);
}

BOOST_AUTO_TEST_SUITE_END();


BOOST_FIXTURE_TEST_SUITE ( byHandler, FileBased );

BOOST_AUTO_TEST_CASE( optionals_areset2_json )
{
	verifyByHelper<TestModule::OptionalsPtr, Slicer::JsonValueDeserializer, Slicer::JsonValueSerializer, json::Value>("optionals-areset2.json", readJson, writeJson, freeJson, checkOptionals_areset);
}

BOOST_AUTO_TEST_CASE( optionals_areset_xml )
{
	verifyByHelper<TestModule::OptionalsPtr, Slicer::XmlDocumentDeserializer, Slicer::XmlDocumentSerializer, xmlpp::Document *>("optionals-areset.xml", readXml, writeXml, freeXml, checkOptionals_areset);
}

BOOST_AUTO_TEST_CASE( simple_complete_validator )
{
	verifyByHelper<TestModule::IsoDate, Slicer::XmlDocumentDeserializer, Slicer::XmlDocumentSerializer, xmlpp::Document *>("isodate.xml", readXml, writeXml, freeXml);
}

BOOST_AUTO_TEST_CASE( missingConversion )
{
	auto in = json::parseValue("{\"conv\": \"2016-06-30 12:34:56\"}");
	BOOST_REQUIRE_THROW((
		Slicer::DeserializeAny<Slicer::JsonValueDeserializer, TestModule2::MissingConvPtr>(in)
	), Slicer::NoConversionFound);

	TestModule2::MissingConvPtr obj = new TestModule2::MissingConv("2016-06-30 12:34:56");
	json::Value v;
	BOOST_REQUIRE_THROW(
		Slicer::SerializeAny<Slicer::JsonValueSerializer>(obj, v),
		Slicer::NoConversionFound);
}

BOOST_AUTO_TEST_CASE( conversion )
{
	auto in = json::parseValue("{\"conv\": \"2016-06-30 12:34:56\"}");
	auto obj = Slicer::DeserializeAny<Slicer::JsonValueDeserializer, TestModule2::ConvPtr>(in);
	BOOST_REQUIRE_EQUAL("2016-06-30 12:34:56", obj->conv);

	json::Value v;
	Slicer::SerializeAny<Slicer::JsonValueSerializer>(obj, v);
	BOOST_REQUIRE_EQUAL("2016-06-30 12:34:56",
			boost::get<json::String>(*boost::get<json::Object>(v)["conv"]));
}

BOOST_FIXTURE_TEST_SUITE ( compatWrapper, FileBased );

BOOST_AUTO_TEST_CASE( any )
{
	BOOST_TEST_CHECKPOINT("Create folders");
	const fs::path tmpf = binDir / "compatWrapper";
	fs::create_directory(tmpf);

	BOOST_TEST_CHECKPOINT("Figure out paths");
	const boost::filesystem::path input = rootDir / "initial" / "builtins.xml";
	const boost::filesystem::path output = tmpf / "builtins.xml";

	BOOST_TEST_CHECKPOINT("Deserialize with wrapper");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	TestModule::BuiltInsPtr object = Slicer::Deserialize<Slicer::XmlFileDeserializer, TestModule::BuiltIns>(input);
#pragma GCC diagnostic pop

	BOOST_TEST_CHECKPOINT("Test object");
	checkBuiltIns_valuesCorrect(object);

	BOOST_TEST_CHECKPOINT("Serialize with wrapper");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	Slicer::Serialize<Slicer::XmlFileSerializer>(object, output);
#pragma GCC diagnostic pop

	BOOST_TEST_CHECKPOINT("Checksum: " << input << " === " << output);
	diff(input, output);
}

BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_CASE( customerModelPartCounters )
{
	BOOST_REQUIRE_EQUAL(7, TestModule::completions);
}

