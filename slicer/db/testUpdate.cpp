#define BOOST_TEST_MODULE db_update
#include "sqlInsertSerializer.h"
#include "sqlSelectDeserializer.h"
#include "sqlUpdateSerializer.h"
#include "testMockCommon.h"
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/test/unit_test.hpp>
#include <common.h>
#include <slicer/slicer.h>
#include <sqlExceptions.h>
#include <testModels.h>
#include <types.h>

using namespace std::literals;

BOOST_GLOBAL_FIXTURE(StandardMockDatabase);

BOOST_FIXTURE_TEST_SUITE(db, ConnectionFixture);

BOOST_AUTO_TEST_CASE(update_builtinsNotFound)
{
	TestModule::BuiltInsPtr ubi = std::make_shared<TestModule::BuiltIns>(false, 5, 17, 64, 129, -1.2, -1.4, "string");
	BOOST_REQUIRE_THROW(Slicer::SerializeAny<Slicer::SqlUpdateSerializer>(ubi, db, "builtins"), Slicer::NoRowsFound);
}

BOOST_AUTO_TEST_CASE(update_builtins)
{
	TestModule::BuiltInsPtr bi1 = std::make_shared<TestModule::BuiltIns>(true, 4, 16, 64, 128, 1.2, 3.4, "text1");
	TestModule::BuiltInsPtr bi2 = std::make_shared<TestModule::BuiltIns>(true, 3, 15, 63, 127, 5.2, 5.4, "text2");
	Slicer::SerializeAny<Slicer::SqlInsertSerializer>(bi1, db, "builtins");
	Slicer::SerializeAny<Slicer::SqlInsertSerializer>(bi2, db, "builtins");

	TestModule::BuiltInsPtr ubi = std::make_shared<TestModule::BuiltIns>(false, 5, 17, 64, 128, -1.2, -1.4, "string");
	Slicer::SerializeAny<Slicer::SqlUpdateSerializer>(ubi, db, "builtins");

	auto sel = db->select("SELECT * FROM builtins ORDER BY mint DESC");
	auto bis = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::BuiltInSeq>(sel.get());
	BOOST_REQUIRE_EQUAL(2, bis.size());
	BOOST_REQUIRE_EQUAL(bis.front()->mbool, ubi->mbool);
	BOOST_REQUIRE_EQUAL(bis.front()->mbyte, ubi->mbyte);
	BOOST_REQUIRE_EQUAL(bis.front()->mshort, ubi->mshort);
	BOOST_REQUIRE_EQUAL(bis.front()->mint, ubi->mint);
	BOOST_REQUIRE_EQUAL(bis.front()->mlong, ubi->mlong);
	BOOST_REQUIRE_EQUAL(bis.front()->mfloat, ubi->mfloat);
	BOOST_REQUIRE_EQUAL(bis.front()->mdouble, ubi->mdouble);
	BOOST_REQUIRE_EQUAL(bis.front()->mstring, ubi->mstring);
	BOOST_REQUIRE_EQUAL(bis.back()->mbool, bi2->mbool);
	BOOST_REQUIRE_EQUAL(bis.back()->mbyte, bi2->mbyte);
	BOOST_REQUIRE_EQUAL(bis.back()->mshort, bi2->mshort);
	BOOST_REQUIRE_EQUAL(bis.back()->mint, bi2->mint);
	BOOST_REQUIRE_EQUAL(bis.back()->mlong, bi2->mlong);
	BOOST_REQUIRE_EQUAL(bis.back()->mfloat, bi2->mfloat);
	BOOST_REQUIRE_EQUAL(bis.back()->mdouble, bi2->mdouble);
	BOOST_REQUIRE_EQUAL(bis.back()->mstring, bi2->mstring);
}

BOOST_AUTO_TEST_CASE(update_builtins_seq)
{
	TestModule::BuiltInSeq ubis {std::make_shared<TestModule::BuiltIns>(false, 5, 17, 64, 128, -1.2, -1.4, "string"),
			std::make_shared<TestModule::BuiltIns>(false, 5, 21, 63, 127, -4.2, -5.4, "string updated")};
	Slicer::SerializeAny<Slicer::SqlUpdateSerializer>(ubis, db, "builtins");

	auto sel = db->select("SELECT * FROM builtins ORDER BY mint");
	auto ubis2 = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::BuiltInSeq>(sel.get());
	BOOST_REQUIRE_EQUAL(2, ubis2.size());
	BOOST_REQUIRE_EQUAL(ubis.front()->mbool, ubis2.back()->mbool);
	BOOST_REQUIRE_EQUAL(ubis.front()->mbyte, ubis2.back()->mbyte);
	BOOST_REQUIRE_EQUAL(ubis.front()->mshort, ubis2.back()->mshort);
	BOOST_REQUIRE_EQUAL(ubis.front()->mint, ubis2.back()->mint);
	BOOST_REQUIRE_EQUAL(ubis.front()->mlong, ubis2.back()->mlong);
	BOOST_REQUIRE_EQUAL(ubis.front()->mfloat, ubis2.back()->mfloat);
	BOOST_REQUIRE_EQUAL(ubis.front()->mdouble, ubis2.back()->mdouble);
	BOOST_REQUIRE_EQUAL(ubis.front()->mstring, ubis2.back()->mstring);
	BOOST_REQUIRE_EQUAL(ubis.back()->mbool, ubis2.front()->mbool);
	BOOST_REQUIRE_EQUAL(ubis.back()->mbyte, ubis2.front()->mbyte);
	BOOST_REQUIRE_EQUAL(ubis.back()->mshort, ubis2.front()->mshort);
	BOOST_REQUIRE_EQUAL(ubis.back()->mint, ubis2.front()->mint);
	BOOST_REQUIRE_EQUAL(ubis.back()->mlong, ubis2.front()->mlong);
	BOOST_REQUIRE_EQUAL(ubis.back()->mfloat, ubis2.front()->mfloat);
	BOOST_REQUIRE_EQUAL(ubis.back()->mdouble, ubis2.front()->mdouble);
	BOOST_REQUIRE_EQUAL(ubis.back()->mstring, ubis2.front()->mstring);
}

BOOST_AUTO_TEST_CASE(update_withNulls)
{
	auto sel = db->select("SELECT * FROM builtins ORDER BY mint");
	auto bis = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestDatabase::BuiltInSeq>(sel.get());
	BOOST_REQUIRE_EQUAL(2, bis.size());
	BOOST_REQUIRE_EQUAL("string updated", *bis[0]->mstring);
	BOOST_REQUIRE_EQUAL("string", *bis[1]->mstring);
	bis[0]->mstring = "not null"s;
	bis[1]->mstring = Ice::optional<std::string>();
	bis[0]->mfloat = Ice::optional<Ice::Float>();
	bis[1]->mbyte = Ice::optional<Ice::Byte>();
	bis[0]->mshort = Ice::optional<Ice::Short>();
	bis[1]->mdouble = Ice::optional<Ice::Double>();
	BOOST_TEST_CHECKPOINT("Do update");
	Slicer::SerializeAny<Slicer::SqlUpdateSerializer>(bis, db, "builtins");
	auto bis2 = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestDatabase::BuiltInSeq>(sel.get());
	BOOST_REQUIRE(bis2[0]->mstring);
	BOOST_REQUIRE(!bis2[1]->mstring);
	BOOST_REQUIRE(bis2[0]->mbyte);
	BOOST_REQUIRE(!bis2[1]->mbyte);
	BOOST_REQUIRE(!bis2[0]->mfloat);
	BOOST_REQUIRE(bis2[1]->mfloat);
}

BOOST_AUTO_TEST_CASE(update_unsupportedModel)
{
	TestModule::ClassMap cm;
	BOOST_REQUIRE_THROW(
			Slicer::SerializeAny<Slicer::SqlUpdateSerializer>(cm, db, "converted"), Slicer::UnsupportedModelType);
}

BOOST_AUTO_TEST_SUITE_END();
