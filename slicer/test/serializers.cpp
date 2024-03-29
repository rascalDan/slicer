#define BOOST_TEST_MODULE execute_serializers
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include "classes.h"
#include "classtype.h"
#include "collections.h"
#include "common.h"
#include "conversions.h"
#include "enums.h"
#include "functions.h"
#include "helpers.h"
#include "inheritance.h"
#include "optionals.h"
#include "structs.h"
#include "xml/serializer.h"
#include <Ice/Config.h>
#include <Ice/Optional.h>
#include <boost/test/unit_test_log.hpp>
#include <definedDirs.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <glibmm/ustring.h>
#include <json.h>
#include <json/serializer.h>
#include <jsonpp.h>
#include <libxml++/document.h>
#include <libxml2/libxml/parser.h>
#include <locals.h>
#include <map>
#include <memory>
#include <slicer/modelParts.h>
#include <slicer/modelPartsTypes.h>
#include <slicer/slicer.h>
#include <sstream>
#include <string>
#include <types.h>
#include <utility>
#include <vector>
#include <xml.h>
// IWYU pragma: no_forward_declare Slicer::InvalidEnumerationSymbol

#ifdef SLICER_MODELPARTSTYPES_IMPL_H
#	error Client code should NOT need to pull in implementation header
#endif

namespace fs = std::filesystem;

// LCOV_EXCL_START
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestModule::ClassMap::iterator)
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestModule::SomeNumbers)

namespace std {
	template<typename T>
	ostream &
	operator<<(ostream & s, const Ice::optional<T> &)
	{
		return s;
	}
}

// LCOV_EXCL_STOP

namespace {
	class FileBased {
	public:
		template<typename T, typename DeserializerIn>
		void
		verifyByFile(const fs::path & infile, const std::function<void(const T &)> & check = nullptr)
		{
			verifyByFile<T, DeserializerIn>(infile, infile, check);
		}

		template<typename T, typename DeserializerIn>
		void
		verifyByFile(const fs::path & infile, const fs::path & expOutFile,
				const std::function<void(const T &)> & check = nullptr)
		{
			const fs::path input = rootDir / "initial" / infile;
			const fs::path expected = rootDir / "initial" / expOutFile;
			const fs::path tmpf = binDir / "byFile";
			fs::create_directory(tmpf);
			const fs::path output = tmpf / infile;
			const fs::path outputJson = tmpf / fs::path(infile).replace_extension("json");
			const fs::path outputXml = tmpf / fs::path(infile).replace_extension("xml");

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
			diff(expected, output);
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
		BOOST_REQUIRE_CLOSE(bt->mdouble, 3.0625, 1);
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
		BOOST_REQUIRE_CLOSE(bt->mdouble, -3.0625, 1);
		BOOST_REQUIRE_EQUAL(bt->mstring, "-Sample text-");
	}

	void
	checkInherits_types(const TestModule::InheritanceContPtr & i)
	{
		BOOST_REQUIRE(i->b);
		BOOST_REQUIRE(std::dynamic_pointer_cast<TestModule::D1>(i->b));
		BOOST_REQUIRE_EQUAL(std::dynamic_pointer_cast<TestModule::D1>(i->b)->a, 1);
		BOOST_REQUIRE_EQUAL(std::dynamic_pointer_cast<TestModule::D1>(i->b)->b, 2);
		BOOST_REQUIRE_EQUAL(i->bs.size(), 3);
		BOOST_REQUIRE(i->bs[0]);
		BOOST_REQUIRE(std::dynamic_pointer_cast<TestModule::D2>(i->bs[0]));
		BOOST_REQUIRE_EQUAL(std::dynamic_pointer_cast<TestModule::D2>(i->bs[0])->a, 1);
		BOOST_REQUIRE_EQUAL(std::dynamic_pointer_cast<TestModule::D2>(i->bs[0])->c, 100);
		BOOST_REQUIRE(i->bs[1]);
		BOOST_REQUIRE(std::dynamic_pointer_cast<TestModule::D3>(i->bs[1]));
		BOOST_REQUIRE_EQUAL(std::dynamic_pointer_cast<TestModule::D3>(i->bs[1])->a, 2);
		BOOST_REQUIRE_EQUAL(std::dynamic_pointer_cast<TestModule::D3>(i->bs[1])->c, 100);
		BOOST_REQUIRE_EQUAL(std::dynamic_pointer_cast<TestModule::D3>(i->bs[1])->d, 200);
		BOOST_REQUIRE(i->bs[2]);
		BOOST_REQUIRE_EQUAL(i->bs[2]->a, 3);
		BOOST_REQUIRE(!std::dynamic_pointer_cast<TestModule::D1>(i->bs[2]));
		BOOST_REQUIRE(!std::dynamic_pointer_cast<TestModule::D2>(i->bs[2]));
		BOOST_REQUIRE(!std::dynamic_pointer_cast<TestModule::D3>(i->bs[2]));
		BOOST_REQUIRE_EQUAL(i->bm.size(), 3);
		BOOST_REQUIRE(std::dynamic_pointer_cast<TestModule::D1>(i->bm.find(10)->second));
		BOOST_REQUIRE(std::dynamic_pointer_cast<TestModule::D3>(i->bm.find(12)->second));
		BOOST_REQUIRE(!std::dynamic_pointer_cast<TestModule::D1>(i->bm.find(14)->second));
		BOOST_REQUIRE(!std::dynamic_pointer_cast<TestModule::D2>(i->bm.find(14)->second));
		BOOST_REQUIRE(!std::dynamic_pointer_cast<TestModule::D3>(i->bm.find(14)->second));
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
		BOOST_REQUIRE(*opts->optClass);
		BOOST_REQUIRE_EQUAL((*opts->optClass)->dt.year, 2017);
		BOOST_REQUIRE_EQUAL((*opts->optClass)->dt.month, 9);
		BOOST_REQUIRE_EQUAL((*opts->optClass)->dt.day, 7);
		BOOST_REQUIRE_EQUAL((*opts->optClass)->dt.hour, 20);
		BOOST_REQUIRE_EQUAL((*opts->optClass)->dt.minute, 40);
		BOOST_REQUIRE_EQUAL((*opts->optClass)->dt.second, 30);
		BOOST_REQUIRE_EQUAL((*opts->optClass)->date.year, 2017);
		BOOST_REQUIRE_EQUAL((*opts->optClass)->date.month, 9);
		BOOST_REQUIRE_EQUAL((*opts->optClass)->date.day, 7);
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
		BOOST_REQUIRE_EQUAL(se->one, TestModule::SomeNumbers::Ten);
		BOOST_REQUIRE_EQUAL(se->two, TestModule::SomeNumbers::FiftyFive);
	}

	void
	checkSomeNumbers(const TestModule::SomeNumbers & sn)
	{
		BOOST_REQUIRE_EQUAL(sn, TestModule::SomeNumbers::FiftyFive);
	}

	void
	attributeMap(const TestXml::Maps & s)
	{
		BOOST_REQUIRE_EQUAL(3, s.amap.size());
		BOOST_REQUIRE_EQUAL("one", s.amap.find("a")->second);
		BOOST_REQUIRE_EQUAL("two", s.amap.find("b")->second);
		BOOST_REQUIRE_EQUAL("three", s.amap.find("c")->second);
	}

	void
	elementMap(const TestXml::Maps & s)
	{
		BOOST_REQUIRE_EQUAL(3, s.emap.size());
		BOOST_REQUIRE_EQUAL("one", s.emap.find("a")->second);
		BOOST_REQUIRE_EQUAL("two", s.emap.find("b")->second);
		BOOST_REQUIRE_EQUAL("three", s.emap.find("c")->second);
		BOOST_REQUIRE_EQUAL(3, s.rmap.size());
		BOOST_REQUIRE_EQUAL(1, s.rmap.find("a")->second.Id);
		BOOST_REQUIRE_EQUAL(2, s.rmap.find("b")->second.Id);
		BOOST_REQUIRE_EQUAL(3, s.rmap.find("c")->second.Id);
		BOOST_REQUIRE_EQUAL("one", s.rmap.find("a")->second.Name);
		BOOST_REQUIRE_EQUAL("two", s.rmap.find("b")->second.Name);
		BOOST_REQUIRE_EQUAL("three", s.rmap.find("c")->second.Name);
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
}

BOOST_FIXTURE_TEST_SUITE(byFile, FileBased)

BOOST_AUTO_TEST_CASE(builtins_xml)
{
	verifyByFile<TestModule::BuiltInsPtr, Slicer::XmlFileDeserializer>("builtins.xml", checkBuiltIns_valuesCorrect);
}

BOOST_AUTO_TEST_CASE(structtype_xml)
{
	verifyByFile<TestModule::StructType, Slicer::XmlFileDeserializer>("struct.xml", checkStruct);
}

BOOST_AUTO_TEST_CASE(structtype_json)
{
	verifyByFile<TestModule::StructType, Slicer::JsonFileDeserializer>("struct2.json", checkStruct);
}

BOOST_AUTO_TEST_CASE(simplestring_xml)
{
	verifyByFile<std::string, Slicer::XmlFileDeserializer>("string.xml", [](const auto & s) {
		BOOST_REQUIRE_EQUAL("test string", s);
	});
}

BOOST_AUTO_TEST_CASE(simpleint_xml)
{
	verifyByFile<Ice::Int, Slicer::XmlFileDeserializer>("int.xml", [](const auto & i) {
		BOOST_REQUIRE_EQUAL(27, i);
	});
}

BOOST_AUTO_TEST_CASE(simplestring_json)
{
	verifyByFile<std::string, Slicer::JsonFileDeserializer>("string2.json", [](const auto & s) {
		BOOST_REQUIRE_EQUAL("test string", s);
	});
}

BOOST_AUTO_TEST_CASE(simpleint_json)
{
	verifyByFile<Ice::Int, Slicer::JsonFileDeserializer>("int2.json", [](const auto & i) {
		BOOST_REQUIRE_EQUAL(27, i);
	});
}

BOOST_AUTO_TEST_CASE(complexClass_xmlattrAndText)
{
	verifyByFile<TestXml::EntityRef, Slicer::XmlFileDeserializer>("entityref.xml", checkEntityRef);
}

BOOST_AUTO_TEST_CASE(sequenceOfClass_xml)
{
	verifyByFile<TestModule::Classes, Slicer::XmlFileDeserializer>("seqOfClass.xml", checkSeqOfClass);
}

BOOST_AUTO_TEST_CASE(sequenceOfClass_json)
{
	verifyByFile<TestModule::Classes, Slicer::JsonFileDeserializer>("seqOfClass2.json", checkSeqOfClass);
}

BOOST_AUTO_TEST_CASE(optionals_notset_xml)
{
	verifyByFile<TestModule::OptionalsPtr, Slicer::XmlFileDeserializer>("optionals-notset.xml", checkOptionals_notset);
}

BOOST_AUTO_TEST_CASE(optionals_areset_xml)
{
	verifyByFile<TestModule::OptionalsPtr, Slicer::XmlFileDeserializer>("optionals-areset.xml", checkOptionals_areset);
}

BOOST_AUTO_TEST_CASE(inherit_a_xml)
{
	verifyByFile<TestModule::InheritanceContPtr, Slicer::XmlFileDeserializer>("inherit-a.xml");
}

BOOST_AUTO_TEST_CASE(inherit_b_xml)
{
	verifyByFile<TestModule::InheritanceContPtr, Slicer::XmlFileDeserializer>("inherit-b.xml", checkInherits_types);
}

BOOST_AUTO_TEST_CASE(conv_datetime_xml)
{
	verifyByFile<TestModule::DateTimeContainerPtr, Slicer::XmlFileDeserializer>("conv-datetime.xml");
}

BOOST_AUTO_TEST_CASE(builtins2_json)
{
	verifyByFile<TestModule::BuiltInsPtr, Slicer::JsonFileDeserializer>("builtins2.json", checkBuiltIns_valuesCorrect);
}

BOOST_AUTO_TEST_CASE(builtins3_json)
{
	verifyByFile<TestModule::BuiltInsPtr, Slicer::JsonFileDeserializer>("builtins3.json", checkBuiltIns3_valuesCorrect);
}

BOOST_AUTO_TEST_CASE(optionals_areset2_json)
{
	verifyByFile<TestModule::OptionalsPtr, Slicer::JsonFileDeserializer>(
			"optionals-areset2.json", checkOptionals_areset);
}

BOOST_AUTO_TEST_CASE(inherit_c_json)
{
	verifyByFile<TestModule::InheritanceContPtr, Slicer::JsonFileDeserializer>("inherit-c.json", checkInherits_types);
}

BOOST_AUTO_TEST_CASE(inherit_d_json)
{
	verifyByFile<TestModule::InheritanceCont2Ptr, Slicer::JsonFileDeserializer>("inherit-d.json");
}

BOOST_AUTO_TEST_CASE(inherit_mapped_json)
{
	verifyByFile<TestModule::InheritanceContMappedPtr, Slicer::JsonFileDeserializer>("inherit-mapped.json");
}

BOOST_AUTO_TEST_CASE(xml_attribute_xml)
{
	verifyByFile<TestModule::ClassClassPtr, Slicer::XmlFileDeserializer>("xmlattr.xml");
}

BOOST_AUTO_TEST_CASE(xml_barecontainers_xml)
{
	verifyByFile<TestXml::BareContainers, Slicer::XmlFileDeserializer>("bare.xml", checkBare);
}

BOOST_AUTO_TEST_CASE(xml_classOfEnums_xml)
{
	verifyByFile<TestModule::SomeEnumsPtr, Slicer::XmlFileDeserializer>("someenums.xml", checkSomeEnums);
}

BOOST_AUTO_TEST_CASE(xml_rootEnums_xml)
{
	verifyByFile<TestModule::SomeNumbers, Slicer::XmlFileDeserializer>("enum.xml", checkSomeNumbers);
}

BOOST_AUTO_TEST_CASE(xml_attributemap_xml)
{
	verifyByFile<TestXml::Maps, Slicer::XmlFileDeserializer>("attributemap.xml", attributeMap);
}

BOOST_AUTO_TEST_CASE(xml_elementmap_xml)
{
	verifyByFile<TestXml::Maps, Slicer::XmlFileDeserializer>("elementmap.xml", elementMap);
}

BOOST_AUTO_TEST_CASE(json_rootEnums_json)
{
	verifyByFile<TestModule::SomeNumbers, Slicer::JsonFileDeserializer>("enum2.json", checkSomeNumbers);
}

BOOST_AUTO_TEST_CASE(json_objectmap)
{
	verifyByFile<TestJson::Properties, Slicer::JsonFileDeserializer>("objectmap.json", checkObjectMap);
}

BOOST_AUTO_TEST_CASE(json_objectmapMember)
{
	verifyByFile<TestJson::HasProperitiesPtr, Slicer::JsonFileDeserializer>(
			"objectmapMember.json", checkObjectMapMember);
}

BOOST_AUTO_TEST_CASE(json_localClass)
{
	verifyByFile<Locals::LocalClassPtr, Slicer::JsonFileDeserializer>("localClass.json");
}

BOOST_AUTO_TEST_CASE(json_localSub2Class)
{
	verifyByFile<Locals::LocalClassPtr, Slicer::JsonFileDeserializer>("localSub2Class.json");
}

BOOST_AUTO_TEST_CASE(json_localSubClass)
{
	verifyByFile<Locals::LocalSubClassPtr, Slicer::JsonFileDeserializer>("localSub2Class.json");
}

BOOST_AUTO_TEST_CASE(json_simpleArray)
{
	verifyByFile<TestModule::SimpleSeq, Slicer::JsonFileDeserializer>("simpleArray1.json");
}

BOOST_AUTO_TEST_CASE(xml_simpleArray)
{
	verifyByFile<TestModule::SimpleSeq, Slicer::XmlFileDeserializer>("simpleArray2.xml");
}

BOOST_AUTO_TEST_CASE(json_emptyToNull)
{
	verifyByFile<TestModule::Optionals2Ptr, Slicer::JsonFileDeserializer>("optionals2.json", [](const auto & o) {
		BOOST_REQUIRE(o);
		BOOST_REQUIRE(!o->optConverted);
		BOOST_REQUIRE_EQUAL(o->nonOptConverted, 4);
	});
}

BOOST_AUTO_TEST_CASE(json_emptyToNull_withValue)
{
	verifyByFile<TestModule::Optionals2Ptr, Slicer::JsonFileDeserializer>("optionals3.json", [](const auto & o) {
		BOOST_REQUIRE(o);
		BOOST_REQUIRE(o->optConverted);
		BOOST_REQUIRE_EQUAL(*o->optConverted, 10);
		BOOST_REQUIRE_EQUAL(o->nonOptConverted, 4);
	});
}

BOOST_AUTO_TEST_CASE(json_emptyToNull_omitted)
{
	verifyByFile<TestModule::Optionals2Ptr, Slicer::JsonFileDeserializer>(
			"optionals5.json", "optionals2.json", [](const auto & o) {
				BOOST_REQUIRE(o);
				BOOST_REQUIRE(!o->optConverted);
				BOOST_REQUIRE_EQUAL(o->nonOptConverted, 4);
			});
}

BOOST_AUTO_TEST_CASE(json_streams)
{
	const auto tmpf = binDir / "byStream";
	const auto inFile = rootDir / "initial" / "inherit-c.json";
	const auto outFile = tmpf / "streamout.json";
	std::filesystem::create_directories(tmpf);
	{
		std::ifstream in(inFile.string());
		auto d = Slicer::DeserializeAny<Slicer::JsonStreamDeserializer, TestModule::InheritanceContPtr>(in);
		checkInherits_types(d);
		std::ofstream out(outFile.string());
		Slicer::SerializeAny<Slicer::JsonStreamSerializer>(d, out);
	}
	diff(inFile, outFile);
}

BOOST_AUTO_TEST_CASE(xml_streams)
{
	const auto tmpf = binDir / "byStream";
	const auto inFile = rootDir / "initial" / "inherit-b.xml";
	const auto outFile = tmpf / "streamout.xml";
	fs::create_directories(tmpf);
	{
		std::ifstream in(inFile.string());
		auto d = Slicer::DeserializeAny<Slicer::XmlStreamDeserializer, TestModule::InheritanceContPtr>(in);
		checkInherits_types(d);
		std::ofstream out(outFile.string());
		Slicer::SerializeAny<Slicer::XmlStreamSerializer>(d, out);
	}
	diff(inFile, outFile);
}

BOOST_AUTO_TEST_CASE(invalid_enum)
{
	Slicer::JsonFileDeserializer jdeserializer {rootDir / "initial" / "invalidEnum.json"};
	BOOST_REQUIRE_THROW(std::ignore = Slicer::DeserializeAnyWith<TestModule::SomeNumbers>(jdeserializer),
			Slicer::InvalidEnumerationSymbol);

	Slicer::XmlFileDeserializer xdeserializer {rootDir / "initial" / "invalidEnum.xml"};
	BOOST_REQUIRE_THROW(std::ignore = Slicer::DeserializeAnyWith<TestModule::SomeNumbers>(xdeserializer),
			Slicer::InvalidEnumerationSymbol);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(missingConversion)
{
	auto in = json::parseValue(R"J({"conv": "2016-06-30 12:34:56"})J");
	BOOST_REQUIRE_THROW(
			(std::ignore = Slicer::DeserializeAny<Slicer::JsonValueDeserializer, TestModule2::MissingConvPtr>(in)),
			Slicer::NoConversionFound);

	auto obj = std::make_shared<TestModule2::MissingConv>("2016-06-30 12:34:56");
	BOOST_REQUIRE_THROW(Slicer::SerializeAny<Slicer::JsonValueSerializer>(obj), Slicer::NoConversionFound);
}

BOOST_AUTO_TEST_CASE(conversion)
{
	auto in = json::parseValue(R"J({"conv": "2016-06-30 12:34:56"})J");
	auto obj = Slicer::DeserializeAny<Slicer::JsonValueDeserializer, TestModule2::ConvPtr>(in);
	BOOST_REQUIRE_EQUAL("2016-06-30 12:34:56", obj->conv);

	std::stringstream out;
	Slicer::SerializeAny<Slicer::JsonStreamSerializer>(obj, out);
	BOOST_REQUIRE_EQUAL(R"J({"conv":"2016-06-30 12:34:56"})J", out.view());
}

BOOST_AUTO_TEST_CASE(DeserializeJsonAbstractEmpty)
{
	auto in = json::parseValue(R"J({ "obj": null })J");
	auto obj = Slicer::DeserializeAny<Slicer::JsonValueDeserializer, Functions::SFuncs>(in);
	BOOST_REQUIRE(!obj.obj);
}

BOOST_AUTO_TEST_CASE(DeserializeJsonAbstractDefault)
{
	auto in = json::parseValue(R"J({ "obj": {} })J");
	BOOST_CHECK_THROW((std::ignore = Slicer::DeserializeAny<Slicer::JsonValueDeserializer, Functions::SFuncs>(in)),
			Slicer::AbstractClassException);
}

BOOST_AUTO_TEST_CASE(DeserializeJsonAbstractImpl)
{
	auto in = json::parseValue(R"J({ "obj": {"slicer-typeid": "::Functions::FuncsSub", "Test-Value": "value"} })J");
	auto obj = Slicer::DeserializeAny<Slicer::JsonValueDeserializer, Functions::SFuncs>(in);
	BOOST_REQUIRE(obj.obj);
	auto impl = std::dynamic_pointer_cast<Functions::FuncsSub>(obj.obj);
	BOOST_REQUIRE(impl);
	BOOST_CHECK_EQUAL("value", impl->testVal);
}

BOOST_AUTO_TEST_CASE(DeserializeXmlAbstractEmpty)
{
	std::stringstream in("<SFuncs/>");
	auto obj = Slicer::DeserializeAny<Slicer::XmlStreamDeserializer, Functions::SFuncs>(in);
	BOOST_REQUIRE(!obj.obj);
}

BOOST_AUTO_TEST_CASE(DeserializeXmlAbstractDefault)
{
	std::stringstream in("<SFuncs><obj/></SFuncs>");
	BOOST_CHECK_THROW((std::ignore = Slicer::DeserializeAny<Slicer::XmlStreamDeserializer, Functions::SFuncs>(in)),
			Slicer::AbstractClassException);
}

BOOST_AUTO_TEST_CASE(DeserializeXmlAbstractImpl)
{
	std::stringstream in(R"X(
			<SFuncs>
				<obj slicer-typeid="::Functions::FuncsSub">
					<Test-Value>value</Test-Value>
				</obj>
			</SFuncs>)X");
	auto obj = Slicer::DeserializeAny<Slicer::XmlStreamDeserializer, Functions::SFuncs>(in);
	BOOST_REQUIRE(obj.obj);
	auto impl = std::dynamic_pointer_cast<Functions::FuncsSub>(obj.obj);
	BOOST_REQUIRE(impl);
	BOOST_CHECK_EQUAL("value", impl->testVal);
}

BOOST_AUTO_TEST_CASE(SerializeJsonClassMap)
{
	TestModule::ClassMap d;
	d[1] = std::make_shared<TestModule::ClassType>(1, 2);
	d[2] = std::make_shared<TestModule::ClassType>(3, 4);
	d[3] = std::make_shared<TestModule::ClassType>(5, 6);
	std::stringstream out;
	Slicer::SerializeAnyWith(d, Slicer::JsonStreamSerializer {out});
	BOOST_CHECK_EQUAL(out.view(),
			R"([{"key":1,"value":{"a":1,"b":2}},{"key":2,"value":{"a":3,"b":4}},{"key":3,"value":{"a":5,"b":6}}])");
}

BOOST_AUTO_TEST_CASE(DeserializeXmlIncorrectSeqElementName)
{
	std::stringstream in(R"X(
			<Classes>
				<obj/>
			</Classes>)X");
	BOOST_CHECK_THROW((std::ignore = Slicer::DeserializeAny<Slicer::XmlStreamDeserializer, TestModule::BuiltInSeq>(in)),
			Slicer::IncorrectElementName);
}

BOOST_AUTO_TEST_CASE(customerModelPartCounters)
{
	BOOST_REQUIRE_EQUAL(22, TestModule::completions);
}

BOOST_AUTO_TEST_CASE(enum_lookups)
{
	BOOST_CHECK_EQUAL("One", Slicer::ModelPartForEnum<TestModule::SomeNumbers>::lookup(TestModule::SomeNumbers::One));
	BOOST_CHECK_EQUAL(TestModule::SomeNumbers::One, Slicer::ModelPartForEnum<TestModule::SomeNumbers>::lookup("One"));
}

BOOST_AUTO_TEST_CASE(sequence_element_in_same_slice_link_bug)
{
	// Link error when sequence element type defined in same slice.
	BOOST_CHECK_NO_THROW(
			Slicer::ModelPart::Make<Slicer::ModelPartForSequence<TestModule::Classes>>(nullptr, [](auto &&) {}));
	BOOST_CHECK_NO_THROW(
			Slicer::ModelPart::Make<Slicer::ModelPartForSequence<TestModule::Dates>>(nullptr, [](auto &&) {}));
}

BOOST_AUTO_TEST_CASE(typeid_specifies_same)
{
	std::ifstream in {rootDir / "initial/inherit-same.xml"};
	auto d1 = Slicer::DeserializeAny<Slicer::XmlStreamDeserializer, TestModule::D1Ptr>(in);
	BOOST_REQUIRE(d1);
	BOOST_CHECK_EQUAL(d1->a, 3);
	auto d1i = *d1;
	BOOST_CHECK_EQUAL(typeid(d1i).name(), typeid(TestModule::D1).name());
}

BOOST_DATA_TEST_CASE(typeid_specifies_bad,
		boost::unit_test::data::make<std::filesystem::path>({
				"inherit-base.xml",
				"inherit-wronghier.xml",
		}),
		path)
{
	std::ifstream in {rootDir / "initial" / path};
	BOOST_CHECK_THROW(std::ignore = (Slicer::DeserializeAny<Slicer::XmlStreamDeserializer, TestModule::D12Ptr>(in)),
			Slicer::IncorrectType);
}

BOOST_DATA_TEST_CASE(typeid_specifies_no_such_type,
		boost::unit_test::data::make<std::filesystem::path>({
				"inherit-nosuchtype.xml",
		}),
		path)
{
	std::ifstream in {rootDir / "initial" / path};
	BOOST_CHECK_THROW(std::ignore = (Slicer::DeserializeAny<Slicer::XmlStreamDeserializer, TestModule::D12Ptr>(in)),
			Slicer::UnknownType);
}

BOOST_AUTO_TEST_CASE(dict_with_name_overrides)
{
	auto res = Slicer::DeserializeAny<Slicer::XmlFileDeserializer, TestModule::StructMapNamed>(
			rootDir / "initial/dictNamed.xml");
	BOOST_REQUIRE_EQUAL(res.size(), 2);
}
