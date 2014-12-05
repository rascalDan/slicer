#define BOOST_TEST_MODULE execute_serializers
#include <boost/test/unit_test.hpp>

#include <slicer/parser.h>
#include <slicer/slicer.h>
#include <slicer/modelParts.h>
#include <xml/serializer.h>
#include <libxml2/libxml/parser.h>
#include <json/serializer.h>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <types.h>
#include <misc.h>
#include <fstream>
#include "helpers.h"
#include "fileStructure.h"

namespace fs = boost::filesystem;

BOOST_TEST_DONT_PRINT_LOG_VALUE ( TestModule::ClassMap::const_iterator )

class FileBased : public FileStructure {
	public:
		template<typename T, typename DeserializerIn>
		void
		verifyByFile(const fs::path & infile, const boost::function<void(const T &)> & check = NULL)
		{
			const fs::path input = root / "initial" / infile;
			const fs::path tmpf = tmp / "byFile";
			fs::create_directory(tmpf);
			const fs::path output = tmpf / infile;
			const fs::path outputJson = tmpf / fs::change_extension(infile, "json");
			const fs::path outputXml = tmpf / fs::change_extension(infile, "xml");

			BOOST_TEST_CHECKPOINT("Deserialize: " << input);
			IceInternal::Handle<T> p = Slicer::Deserialize<DeserializerIn, T>(input);
			
			if (check) {
				BOOST_TEST_CHECKPOINT("Check1: " << input);
				check(*p);
			}

			BOOST_TEST_CHECKPOINT("Serialize " << input << " -> " << outputJson);
			Slicer::Serialize<Slicer::JsonFileSerializer>(p, outputJson);

			BOOST_TEST_CHECKPOINT("Serialize " << input << " -> " << outputXml);
			Slicer::Serialize<Slicer::XmlFileSerializer>(p, outputXml);

			if (check) {
				BOOST_TEST_CHECKPOINT("Check2: " << input);
				check(*p);
			}

			BOOST_TEST_CHECKPOINT("Checksum: " << input << " === " << output);
			system(stringbf("diff -w %s %s", input, output));
		}

		template<typename T, typename Deserializer, typename Serializer, typename Internal>
		void
		verifyByHelper(const fs::path & infile,
				const boost::function<Internal(const fs::path &)> & in,
				const boost::function<void(const Internal &, const fs::path &)> & out,
				const boost::function<void(Internal &)> & ifree,
				const boost::function<void(const T &)> & check = NULL)
		{
			const fs::path input = root / "initial" / infile;
			const fs::path tmph = tmp / "byHandler";
			fs::create_directory(tmph);
			const fs::path output = tmph / infile;

			BOOST_TEST_CHECKPOINT("Read: " << input);
			Internal docRead = in(input);

			BOOST_TEST_CHECKPOINT("Deserialize: " << input);
			IceInternal::Handle<T> p = Slicer::Deserialize<Deserializer, T>(docRead);
			ifree(docRead);

			if (check) {
				BOOST_TEST_CHECKPOINT("Check1: " << input);
				check(*p);
			}

			BOOST_TEST_CHECKPOINT("Serialize: " << input);
			Internal docWrite;
			Slicer::Serialize<Serializer>(p, docWrite);

			if (check) {
				BOOST_TEST_CHECKPOINT("Check2: " << input);
				check(*p);
			}

			BOOST_TEST_CHECKPOINT("Write: " << output);
			out(docWrite, output);
			ifree(docWrite);

			BOOST_TEST_CHECKPOINT("Checksum: " << input << " === " << output);
			system(stringbf("diff -w %s %s", input, output));
		}
};

void
checkBuiltIns_valuesCorrect(const TestModule::BuiltIns & bt)
{
	BOOST_REQUIRE_EQUAL(bt.mbool, true);
	BOOST_REQUIRE_EQUAL(bt.mbyte, 4);
	BOOST_REQUIRE_EQUAL(bt.mshort, 40);
	BOOST_REQUIRE_EQUAL(bt.mint, 80);
	BOOST_REQUIRE_EQUAL(bt.mlong, 800);
	BOOST_REQUIRE_EQUAL(bt.mfloat, 3.125);
	BOOST_REQUIRE_EQUAL(bt.mdouble, 3.0625);
	BOOST_REQUIRE_EQUAL(bt.mstring, "Sample text");
}

void
checkInherits_types(const TestModule::InheritanceCont & i)
{
	BOOST_REQUIRE(i.b);
	BOOST_REQUIRE(TestModule::D1Ptr::dynamicCast(i.b));
	BOOST_REQUIRE_EQUAL(TestModule::D1Ptr::dynamicCast(i.b)->a, 1);
	BOOST_REQUIRE_EQUAL(TestModule::D1Ptr::dynamicCast(i.b)->b, 2);
	BOOST_REQUIRE_EQUAL(i.bs.size(), 3);
	BOOST_REQUIRE(i.bs[0]);
	BOOST_REQUIRE(TestModule::D2Ptr::dynamicCast(i.bs[0]));
	BOOST_REQUIRE_EQUAL(TestModule::D2Ptr::dynamicCast(i.bs[0])->a, 1);
	BOOST_REQUIRE_EQUAL(TestModule::D2Ptr::dynamicCast(i.bs[0])->c, 100);
	BOOST_REQUIRE(i.bs[1]);
	BOOST_REQUIRE(TestModule::D3Ptr::dynamicCast(i.bs[1]));
	BOOST_REQUIRE_EQUAL(TestModule::D3Ptr::dynamicCast(i.bs[1])->a, 2);
	BOOST_REQUIRE_EQUAL(TestModule::D3Ptr::dynamicCast(i.bs[1])->c, 100);
	BOOST_REQUIRE_EQUAL(TestModule::D3Ptr::dynamicCast(i.bs[1])->d, 200);
	BOOST_REQUIRE(i.bs[2]);
	BOOST_REQUIRE_EQUAL(i.bs[2]->a, 3);
	BOOST_REQUIRE(!TestModule::D1Ptr::dynamicCast(i.bs[2]));
	BOOST_REQUIRE(!TestModule::D2Ptr::dynamicCast(i.bs[2]));
	BOOST_REQUIRE(!TestModule::D3Ptr::dynamicCast(i.bs[2]));
	BOOST_REQUIRE_EQUAL(i.bm.size(), 3);
	BOOST_REQUIRE(TestModule::D1Ptr::dynamicCast(i.bm.find(10)->second));
	BOOST_REQUIRE(TestModule::D3Ptr::dynamicCast(i.bm.find(12)->second));
	BOOST_REQUIRE(!TestModule::D1Ptr::dynamicCast(i.bm.find(14)->second));
	BOOST_REQUIRE(!TestModule::D2Ptr::dynamicCast(i.bm.find(14)->second));
	BOOST_REQUIRE(!TestModule::D3Ptr::dynamicCast(i.bm.find(14)->second));
}

void
checkOptionals_notset(const TestModule::Optionals & opts)
{
	BOOST_REQUIRE(!opts.optSimple);
	BOOST_REQUIRE(!opts.optStruct);
	BOOST_REQUIRE(!opts.optClass);
	BOOST_REQUIRE(!opts.optSeq);
	BOOST_REQUIRE(!opts.optDict);
}

void
checkOptionals_areset(const TestModule::Optionals & opts)
{
	BOOST_REQUIRE(opts.optSimple);
	BOOST_REQUIRE_EQUAL(opts.optSimple, 4);
	BOOST_REQUIRE(opts.optStruct);
	BOOST_REQUIRE_EQUAL(opts.optStruct->a, 1);
	BOOST_REQUIRE_EQUAL(opts.optStruct->b, 2);
	BOOST_REQUIRE(opts.optClass);
	BOOST_REQUIRE_EQUAL((*opts.optClass)->a, 1);
	BOOST_REQUIRE_EQUAL((*opts.optClass)->b, 2);
	BOOST_REQUIRE_EQUAL(opts.optSeq->size(), 2);
	BOOST_REQUIRE_EQUAL((*opts.optSeq)[0]->a, 3);
	BOOST_REQUIRE_EQUAL((*opts.optSeq)[0]->b, 4);
	BOOST_REQUIRE_EQUAL((*opts.optSeq)[1]->a, 5);
	BOOST_REQUIRE_EQUAL((*opts.optSeq)[1]->b, 6);
	BOOST_REQUIRE(opts.optDict);
	BOOST_REQUIRE_EQUAL(opts.optDict->size(), 2);
	BOOST_REQUIRE_EQUAL(opts.optDict->find(1), opts.optDict->end());
	BOOST_REQUIRE(opts.optDict->find(10) != opts.optDict->end());
	BOOST_REQUIRE_EQUAL(opts.optDict->find(10)->second->a, 11);
	BOOST_REQUIRE_EQUAL(opts.optDict->find(10)->second->b, 12);
	BOOST_REQUIRE(opts.optDict->find(13) != opts.optDict->end());
	BOOST_REQUIRE_EQUAL(opts.optDict->find(13)->second->a, 14);
	BOOST_REQUIRE_EQUAL(opts.optDict->find(13)->second->b, 15);
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
	verifyByFile<TestModule::BuiltIns, Slicer::XmlFileDeserializer>("builtins.xml", checkBuiltIns_valuesCorrect);
}

BOOST_AUTO_TEST_CASE( optionals_notset_xml )
{
	verifyByFile<TestModule::Optionals, Slicer::XmlFileDeserializer>("optionals-notset.xml", checkOptionals_notset);
}

BOOST_AUTO_TEST_CASE( optionals_areset_xml )
{
	verifyByFile<TestModule::Optionals, Slicer::XmlFileDeserializer>("optionals-areset.xml", checkOptionals_areset);
}

BOOST_AUTO_TEST_CASE( inherit_a_xml )
{
	verifyByFile<TestModule::InheritanceCont, Slicer::XmlFileDeserializer>("inherit-a.xml");
}

BOOST_AUTO_TEST_CASE( inherit_b_xml )
{
	verifyByFile<TestModule::InheritanceCont, Slicer::XmlFileDeserializer>("inherit-b.xml", checkInherits_types);
}

BOOST_AUTO_TEST_CASE( conv_datetime_xml )
{
	verifyByFile<TestModule::DateTimeContainer, Slicer::XmlFileDeserializer>("conv-datetime.xml");
}

BOOST_AUTO_TEST_CASE( builtins2_json )
{
	verifyByFile<TestModule::BuiltIns, Slicer::JsonFileDeserializer>("builtins2.json", checkBuiltIns_valuesCorrect);
}

BOOST_AUTO_TEST_CASE( optionals_areset2_json )
{
	verifyByFile<TestModule::Optionals, Slicer::JsonFileDeserializer>("optionals-areset2.json", checkOptionals_areset);
}

BOOST_AUTO_TEST_CASE( inherit_c_json )
{
	verifyByFile<TestModule::InheritanceCont, Slicer::JsonFileDeserializer>("inherit-c.json", checkInherits_types);
}

BOOST_AUTO_TEST_CASE( inherit_d_json )
{
	verifyByFile<TestModule::InheritanceCont2, Slicer::JsonFileDeserializer>("inherit-d.json");
}

BOOST_AUTO_TEST_CASE( inherit_mapped_json )
{
	verifyByFile<TestModule::InheritanceContMapped, Slicer::JsonFileDeserializer>("inherit-mapped.json");
}

BOOST_AUTO_TEST_SUITE_END();


BOOST_FIXTURE_TEST_SUITE ( byHandler, FileBased );

BOOST_AUTO_TEST_CASE( optionals_areset2_json )
{
	verifyByHelper<TestModule::Optionals, Slicer::JsonValueDeserializer, Slicer::JsonValueSerializer, json::Value>("optionals-areset2.json", readJson, writeJson, freeJson, checkOptionals_areset);
}

BOOST_AUTO_TEST_CASE( optionals_areset_xml )
{
	verifyByHelper<TestModule::Optionals, Slicer::XmlDocumentDeserializer, Slicer::XmlDocumentSerializer, xmlpp::Document *>("optionals-areset.xml", readXml, writeXml, freeXml, checkOptionals_areset);
}

BOOST_AUTO_TEST_SUITE_END();
