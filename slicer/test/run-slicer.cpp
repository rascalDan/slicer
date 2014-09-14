#include <slicer/parser.h>
#include <slicer/slicer.h>
#include <slicer/modelParts.h>
#include <xml/serializer.h>
#include <libxml2/libxml/parser.h>
#include <json/serializer.h>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/assert.hpp>
#include <types.h>
#include <misc.h>
#include <fstream>
#include "helpers.h"

namespace fs = boost::filesystem;
#define SHORT(x) boost::numeric_cast< ::Ice::Short >(x)

namespace Slicer {
	boost::posix_time::ptime
	dateTimeToPTime(const ::TestModule::DateTime &)
	{
		throw std::runtime_error("Not implemented");
	}

	::TestModule::DateTime
	ptimeToDateTime(const boost::posix_time::ptime &)
	{
		throw std::runtime_error("Not implemented");
	}

	std::string
	dateTimeToString(const ::TestModule::DateTime & in)
	{
		char buf[BUFSIZ];
		struct tm tm({ in.second, in.minute, in.hour, in.day, in.month, in.year, 0, 0, 0
#ifdef _BSD_SOURCE
				, 0, 0
#endif
				});
		mktime(&tm);
		auto len = strftime(buf, BUFSIZ, "%Y-%b-%d %H:%M:%S", &tm);
		return std::string(buf, len);
	}

	::TestModule::DateTime
	stringToDateTime(const std::string & in)
	{
		struct tm tm;
		memset(&tm, 0, sizeof(struct tm));
		auto end = strptime(in.c_str(), "%Y-%b-%d %H:%M:%S", &tm);
		if (!end || *end) {
			throw std::runtime_error("Invalid date string: " + in);
		}
		return ::TestModule::DateTime({
				SHORT(tm.tm_year), SHORT(tm.tm_mon), SHORT(tm.tm_mday),
				SHORT(tm.tm_hour), SHORT(tm.tm_min), SHORT(tm.tm_sec)});
	}
}

template<typename T, typename SerializerIn>
void
verifyByFile(const fs::path & root, const fs::path & tmp, const fs::path & infile, const boost::function<void(const T &)> & check = NULL)
{
	const fs::path input = root / "initial" / infile;
	const fs::path output = tmp / infile;
	const fs::path outputJson = tmp / fs::change_extension(infile, "json");
	const fs::path outputXml = tmp / fs::change_extension(infile, "xml");

	fprintf(stderr, "%s : Deserialize\n", input.string().c_str());
	IceInternal::Handle<T> p = Slicer::Deserialize<SerializerIn, T>(input);
	if (check) {
		fprintf(stderr, "%s : Check1\n", input.string().c_str());
		check(*p);
	}
	fprintf(stderr, "%s : Serialize -> %s\n", input.string().c_str(), outputJson.string().c_str());
	Slicer::Serialize<Slicer::JsonFile>(p, outputJson);
	fprintf(stderr, "%s : Serialize -> %s\n", input.string().c_str(), outputXml.string().c_str());
	Slicer::Serialize<Slicer::XmlFile>(p, outputXml);
	if (check) {
		fprintf(stderr, "%s : Check2\n", input.string().c_str());
		check(*p);
	}
	fprintf(stderr, "%s : OK\n", input.string().c_str());

	system(stringbf("diff -w %s %s", input, output));
}

template<typename T, typename Serializer, typename Internal>
void
verifyByHelper(const fs::path & root, const fs::path & tmp, const fs::path & infile,
		const boost::function<Internal(const fs::path &)> & in,
		const boost::function<void(const Internal &, const fs::path &)> & out,
		const boost::function<void(Internal &)> & ifree,
		const boost::function<void(const T &)> & check = NULL)
{
	const fs::path input = root / "initial" / infile;
	const fs::path output = tmp / infile;

	fprintf(stderr, "%s : Read\n", input.string().c_str());
	Internal docRead = in(input);

	fprintf(stderr, "%s : Deserialize\n", input.string().c_str());
	IceInternal::Handle<T> p = Slicer::Deserialize<Serializer, T>(docRead);
	ifree(docRead);
	if (check) {
		fprintf(stderr, "%s : Check1\n", input.string().c_str());
		check(*p);
	}

	fprintf(stderr, "%s : Serialize\n", input.string().c_str());
	Internal docWrite;
	Slicer::Serialize<Serializer>(p, docWrite);
	if (check) {
		fprintf(stderr, "%s : Check2\n", input.string().c_str());
		check(*p);
	}

	fprintf(stderr, "%s : Write\n", input.string().c_str());
	out(docWrite, output);
	ifree(docWrite);

	fprintf(stderr, "%s : OK\n", input.string().c_str());

	system(stringbf("diff -w %s %s", input, output));
}

void
checkBuiltIns_valuesCorrect(const TestModule::BuiltIns & bt)
{
	BOOST_ASSERT(bt.mbool);
	BOOST_ASSERT(bt.mbyte == 4);
	BOOST_ASSERT(bt.mshort == 40);
	BOOST_ASSERT(bt.mint == 80);
	BOOST_ASSERT(bt.mlong == 800);
	BOOST_ASSERT(bt.mfloat == 3.125);
	BOOST_ASSERT(bt.mdouble == 3.0625);
	BOOST_ASSERT(bt.mstring == "Sample text");
}

void
checkInherits_types(const TestModule::InheritanceCont & i)
{
	BOOST_ASSERT(i.b);
	BOOST_ASSERT(TestModule::D1Ptr::dynamicCast(i.b));
	BOOST_ASSERT(TestModule::D1Ptr::dynamicCast(i.b)->a == 1);
	BOOST_ASSERT(TestModule::D1Ptr::dynamicCast(i.b)->b == 2);
	BOOST_ASSERT(i.bs.size() == 3);
	BOOST_ASSERT(i.bs[0]);
	BOOST_ASSERT(TestModule::D2Ptr::dynamicCast(i.bs[0]));
	BOOST_ASSERT(TestModule::D2Ptr::dynamicCast(i.bs[0])->a == 1);
	BOOST_ASSERT(TestModule::D2Ptr::dynamicCast(i.bs[0])->c == 100);
	BOOST_ASSERT(i.bs[1]);
	BOOST_ASSERT(TestModule::D3Ptr::dynamicCast(i.bs[1]));
	BOOST_ASSERT(TestModule::D3Ptr::dynamicCast(i.bs[1])->a == 2);
	BOOST_ASSERT(TestModule::D3Ptr::dynamicCast(i.bs[1])->c == 100);
	BOOST_ASSERT(TestModule::D3Ptr::dynamicCast(i.bs[1])->d == 200);
	BOOST_ASSERT(i.bs[2]);
	BOOST_ASSERT(i.bs[2]->a == 3);
	BOOST_ASSERT(!TestModule::D1Ptr::dynamicCast(i.bs[2]));
	BOOST_ASSERT(!TestModule::D2Ptr::dynamicCast(i.bs[2]));
	BOOST_ASSERT(!TestModule::D3Ptr::dynamicCast(i.bs[2]));
	BOOST_ASSERT(i.bm.size() == 3);
	BOOST_ASSERT(TestModule::D1Ptr::dynamicCast(i.bm.find(10)->second));
	BOOST_ASSERT(TestModule::D3Ptr::dynamicCast(i.bm.find(12)->second));
	BOOST_ASSERT(!TestModule::D1Ptr::dynamicCast(i.bm.find(14)->second));
	BOOST_ASSERT(!TestModule::D2Ptr::dynamicCast(i.bm.find(14)->second));
	BOOST_ASSERT(!TestModule::D3Ptr::dynamicCast(i.bm.find(14)->second));
}

void
checkOptionals_notset(const TestModule::Optionals & opts)
{
	BOOST_ASSERT(!opts.optSimple);
	BOOST_ASSERT(!opts.optStruct);
	BOOST_ASSERT(!opts.optClass);
	BOOST_ASSERT(!opts.optSeq);
	BOOST_ASSERT(!opts.optDict);
}

void
checkOptionals_areset(const TestModule::Optionals & opts)
{
	BOOST_ASSERT(opts.optSimple);
	BOOST_ASSERT(opts.optSimple == 4);
	BOOST_ASSERT(opts.optStruct);
	BOOST_ASSERT(opts.optStruct->a == 1);
	BOOST_ASSERT(opts.optStruct->b == 2);
	BOOST_ASSERT(opts.optClass);
	BOOST_ASSERT((*opts.optClass)->a == 1);
	BOOST_ASSERT((*opts.optClass)->b == 2);
	BOOST_ASSERT(opts.optSeq->size() == 2);
	BOOST_ASSERT((*opts.optSeq)[0]->a == 3);
	BOOST_ASSERT((*opts.optSeq)[0]->b == 4);
	BOOST_ASSERT((*opts.optSeq)[1]->a == 5);
	BOOST_ASSERT((*opts.optSeq)[1]->b == 6);
	BOOST_ASSERT(opts.optDict);
	BOOST_ASSERT(opts.optDict->size() == 2);
	BOOST_ASSERT(opts.optDict->find(1) == opts.optDict->end());
	BOOST_ASSERT(opts.optDict->find(10) != opts.optDict->end());
	BOOST_ASSERT(opts.optDict->find(10)->second->a == 11);
	BOOST_ASSERT(opts.optDict->find(10)->second->b == 12);
	BOOST_ASSERT(opts.optDict->find(13) != opts.optDict->end());
	BOOST_ASSERT(opts.optDict->find(13)->second->a == 14);
	BOOST_ASSERT(opts.optDict->find(13)->second->b == 15);
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

int
main(int, char ** argv)
{
	const fs::path me = argv[0];
	const fs::path base = "types";
	const fs::path bjamout = me.parent_path();
	const fs::path root = me.parent_path().parent_path().parent_path().parent_path();
	fprintf(stderr, "root -- %s\n", root.string().c_str());
	const fs::path slice = fs::change_extension(root / base, ".ice");
	fprintf(stderr, "slice - %s\n", slice.string().c_str());

	// Tmp
	const fs::path tmp = root / "bin" / "slicer";
	fprintf(stderr, "tmp --- %s\n", tmp.string().c_str());
	fs::create_directory(tmp);
	const fs::path tmpf = tmp / "byFile";
	fs::create_directory(tmpf);
	const fs::path tmph = tmp / "byHelper";
	fs::create_directory(tmph);

	// Execute
	verifyByFile<TestModule::BuiltIns, Slicer::XmlFile>(root, tmpf, "builtins.xml", checkBuiltIns_valuesCorrect);
	verifyByFile<TestModule::Optionals, Slicer::XmlFile>(root, tmpf, "optionals-notset.xml", checkOptionals_notset);
	verifyByFile<TestModule::Optionals, Slicer::XmlFile>(root, tmpf, "optionals-areset.xml", checkOptionals_areset);
	verifyByFile<TestModule::InheritanceCont, Slicer::XmlFile>(root, tmpf, "inherit-a.xml");
	verifyByFile<TestModule::InheritanceCont, Slicer::XmlFile>(root, tmpf, "inherit-b.xml", checkInherits_types);
	verifyByFile<TestModule::DateTimeContainer, Slicer::XmlFile>(root, tmpf, "conv-datetime.xml");
	verifyByFile<TestModule::BuiltIns, Slicer::JsonFile>(root, tmpf, "builtins2.json", checkBuiltIns_valuesCorrect);
	verifyByFile<TestModule::Optionals, Slicer::JsonFile>(root, tmpf, "optionals-areset2.json", checkOptionals_areset);
	verifyByFile<TestModule::InheritanceCont, Slicer::JsonFile>(root, tmpf, "inherit-c.json", checkInherits_types);

	verifyByHelper<TestModule::Optionals, Slicer::JsonValue, json::Value>(root, tmph, "optionals-areset2.json", readJson, writeJson, freeJson, checkOptionals_areset);
	verifyByHelper<TestModule::Optionals, Slicer::XmlDocument, xmlpp::Document *>(root, tmph, "optionals-areset.xml", readXml, writeXml, freeXml, checkOptionals_areset);

	return 0;
}

