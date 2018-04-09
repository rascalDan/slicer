#define BOOST_TEST_MODULE db_insert
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include "testMockCommon.h"
#include <slicer/slicer.h>
#include "sqlInsertSerializer.h"
#include "sqlSelectDeserializer.h"
#include <types.h>
#include <common.h>
#include <testModels.h>

using namespace std::literals;

// LCOV_EXCL_START
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestModule::DateTime);
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestModule::IsoDate);
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestDatabase::Timespan);
// LCOV_EXCL_STOP

namespace std {
	template<typename T>
	ostream & operator<<(ostream & s, const IceUtil::Optional<T> &) {
		return s;
	}
}

BOOST_GLOBAL_FIXTURE( StandardMockDatabase );

BOOST_FIXTURE_TEST_SUITE(db, ConnectionFixture);

BOOST_AUTO_TEST_CASE( insert_builtins )
{
	TestModule::BuiltInsPtr bi = std::make_shared<TestModule::BuiltIns>(true, 4, 16, 64, 128, 1.2, 3.4, "text");
	Slicer::SerializeAny<Slicer::SqlInsertSerializer>(bi, db, "builtins");
	auto sel = db->select("SELECT * FROM builtins");
	auto bi2 = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::BuiltInsPtr>(sel);
	BOOST_REQUIRE_EQUAL(bi->mbool, bi2->mbool);
	BOOST_REQUIRE_EQUAL(bi->mbyte, bi2->mbyte);
	BOOST_REQUIRE_EQUAL(bi->mshort, bi2->mshort);
	BOOST_REQUIRE_EQUAL(bi->mint, bi2->mint);
	BOOST_REQUIRE_EQUAL(bi->mlong, bi2->mlong);
	BOOST_REQUIRE_EQUAL(bi->mfloat, bi2->mfloat);
	BOOST_REQUIRE_EQUAL(bi->mdouble, bi2->mdouble);
	BOOST_REQUIRE_EQUAL(bi->mstring, bi2->mstring);
}

BOOST_AUTO_TEST_CASE( insert_seq_builtins )
{
	TestModule::BuiltInSeq bis = {
		std::make_shared<TestModule::BuiltIns>(true, 5, 17, 65, 129, 2.3, 4.5, "more text"),
		std::make_shared<TestModule::BuiltIns>(true, 6, 18, 66, 130, 3.4, 5.6, "even more text")
	};
	Slicer::SerializeAny<Slicer::SqlInsertSerializer>(bis, db, "builtins");
	auto sel = db->select("SELECT * FROM builtins ORDER BY mint");
	auto bis2 = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::BuiltInSeq>(sel);
	BOOST_REQUIRE_EQUAL(3, bis2.size());
	BOOST_REQUIRE_EQUAL(bis.back()->mbool, bis2.back()->mbool);
	BOOST_REQUIRE_EQUAL(bis.back()->mbyte, bis2.back()->mbyte);
	BOOST_REQUIRE_EQUAL(bis.back()->mshort, bis2.back()->mshort);
	BOOST_REQUIRE_EQUAL(bis.back()->mint, bis2.back()->mint);
	BOOST_REQUIRE_EQUAL(bis.back()->mlong, bis2.back()->mlong);
	BOOST_REQUIRE_EQUAL(bis.back()->mfloat, bis2.back()->mfloat);
	BOOST_REQUIRE_EQUAL(bis.back()->mdouble, bis2.back()->mdouble);
	BOOST_REQUIRE_EQUAL(bis.back()->mstring, bis2.back()->mstring);
}

BOOST_AUTO_TEST_CASE( autoinsert_seq_builtins )
{
	TestModule::BuiltInSeq bis = {
		std::make_shared<TestModule::BuiltIns>(true, 5, 17, 0, 129, 2.3, 4.5, "more text"),
		std::make_shared<TestModule::BuiltIns>(true, 6, 18, 0, 130, 3.4, 5.6, "even more text")
	};
	Slicer::SerializeAny<Slicer::SqlAutoIdInsertSerializer>(bis, db, "builtins");
	auto sel = db->select("SELECT * FROM builtins WHERE mint IN (1, 2) ORDER BY mint");
	auto bis2 = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::BuiltInSeq>(sel);
	BOOST_REQUIRE_EQUAL(2, bis2.size());
	BOOST_REQUIRE_EQUAL(bis.front()->mint, 0);
	BOOST_REQUIRE_EQUAL(bis.back()->mint, 0);
	BOOST_REQUIRE_EQUAL(bis2.front()->mint, 1);
	BOOST_REQUIRE_EQUAL(bis2.back()->mint, 2);
	BOOST_REQUIRE_EQUAL(bis.back()->mbool, bis2.back()->mbool);
	BOOST_REQUIRE_EQUAL(bis.back()->mbyte, bis2.back()->mbyte);
	BOOST_REQUIRE_EQUAL(bis.back()->mshort, bis2.back()->mshort);
	BOOST_REQUIRE_EQUAL(bis.back()->mlong, bis2.back()->mlong);
	BOOST_REQUIRE_EQUAL(bis.back()->mfloat, bis2.back()->mfloat);
	BOOST_REQUIRE_EQUAL(bis.back()->mdouble, bis2.back()->mdouble);
	BOOST_REQUIRE_EQUAL(bis.back()->mstring, bis2.back()->mstring);
}

BOOST_AUTO_TEST_CASE( fetchinsert_seq_builtins )
{
	TestModule::BuiltInSeq bis = {
		std::make_shared<TestModule::BuiltIns>(true, 5, 17, 0, 129, 2.3, 4.5, "more text"),
		std::make_shared<TestModule::BuiltIns>(true, 6, 18, 0, 130, 3.4, 5.6, "even more text")
	};
	Slicer::SerializeAny<Slicer::SqlFetchIdInsertSerializer>(bis, db, "builtins");
	auto sel = db->select("SELECT * FROM builtins WHERE mint IN (3, 4) ORDER BY mint");
	auto bis2 = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::BuiltInSeq>(sel);
	BOOST_REQUIRE_EQUAL(2, bis2.size());
	BOOST_REQUIRE_EQUAL(bis.front()->mint, 3);
	BOOST_REQUIRE_EQUAL(bis.back()->mint, 4);
	BOOST_REQUIRE_EQUAL(bis2.front()->mint, 3);
	BOOST_REQUIRE_EQUAL(bis2.back()->mint, 4);
	BOOST_REQUIRE_EQUAL(bis.back()->mbool, bis2.back()->mbool);
	BOOST_REQUIRE_EQUAL(bis.back()->mbyte, bis2.back()->mbyte);
	BOOST_REQUIRE_EQUAL(bis.back()->mshort, bis2.back()->mshort);
	BOOST_REQUIRE_EQUAL(bis.back()->mlong, bis2.back()->mlong);
	BOOST_REQUIRE_EQUAL(bis.back()->mfloat, bis2.back()->mfloat);
	BOOST_REQUIRE_EQUAL(bis.back()->mdouble, bis2.back()->mdouble);
	BOOST_REQUIRE_EQUAL(bis.back()->mstring, bis2.back()->mstring);
}

BOOST_AUTO_TEST_CASE( fetchinsert_seq_builtinsWithNulls )
{
	TestDatabase::BuiltInSeq bis = {
		std::make_shared<TestDatabase::BuiltIns>(true, IceUtil::None, 17, 0, 129, 2.3, 4.5, "more text"s),
		std::make_shared<TestDatabase::BuiltIns>(true, 6, 18, 0, 130, 3.4, IceUtil::None, "even more text"s)
	};
	Slicer::SerializeAny<Slicer::SqlFetchIdInsertSerializer>(bis, db, "builtins");
	auto sel = db->select("SELECT * FROM builtins WHERE mint IN (5, 6) ORDER BY mint");
	auto bis2 = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestDatabase::BuiltInSeq>(sel);
	BOOST_REQUIRE_EQUAL(2, bis2.size());
	BOOST_REQUIRE_EQUAL(bis.front()->mint, 5);
	BOOST_REQUIRE_EQUAL(bis.back()->mint, 6);
	BOOST_REQUIRE_EQUAL(bis2.front()->mint, 5);
	BOOST_REQUIRE_EQUAL(bis2.back()->mint, 6);
	BOOST_REQUIRE_EQUAL(bis.back()->mbool, bis2.back()->mbool);
	BOOST_REQUIRE_EQUAL(bis.back()->mbyte, bis2.back()->mbyte);
	BOOST_REQUIRE_EQUAL(bis.back()->mshort, bis2.back()->mshort);
	BOOST_REQUIRE_EQUAL(bis.back()->mlong, bis2.back()->mlong);
	BOOST_REQUIRE_EQUAL(bis.back()->mfloat, bis2.back()->mfloat);
	BOOST_REQUIRE_EQUAL(bis.back()->mdouble, bis2.back()->mdouble);
	BOOST_REQUIRE_EQUAL(bis.back()->mstring, bis2.back()->mstring);
}

BOOST_AUTO_TEST_CASE( insert_converted )
{
	TestDatabase::SpecificTypesPtr st = std::make_shared<TestDatabase::SpecificTypes>(
		TestModule::DateTime {2015, 10, 16, 19, 12, 34},
		TestModule::IsoDate {2015, 10, 16},
		std::make_shared<TestDatabase::Timespan>(1, 2, 3, 4)
	);
	Slicer::SerializeAny<Slicer::SqlInsertSerializer>(st, db, "converted");
	auto sel = db->select("SELECT * FROM converted");
	auto st2 = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestDatabase::SpecificTypesPtr>(sel);
	BOOST_REQUIRE_EQUAL(st->date, st2->date);
	BOOST_REQUIRE_EQUAL(st->dt, st2->dt);
	BOOST_REQUIRE_EQUAL(st->ts->days, st2->ts->days);
	BOOST_REQUIRE_EQUAL(st->ts->hours, st2->ts->hours);
	BOOST_REQUIRE_EQUAL(st->ts->minutes, st2->ts->minutes);
	BOOST_REQUIRE_EQUAL(st->ts->seconds, st2->ts->seconds);
}

BOOST_AUTO_TEST_CASE( insert_unsupportedModel )
{
	TestModule::ClassMap cm;
	BOOST_REQUIRE_THROW(Slicer::SerializeAny<Slicer::SqlInsertSerializer>(cm, db, "converted"), Slicer::UnsupportedModelType);
}

BOOST_AUTO_TEST_SUITE_END();

