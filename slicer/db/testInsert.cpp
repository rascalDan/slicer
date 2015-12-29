#define BOOST_TEST_MODULE db_insert
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <pq-mock.h>
#include <slicer/slicer.h>
#include <definedDirs.h>
#include "sqlInsertSerializer.h"
#include "sqlSelectDeserializer.h"
#include <types.h>
#include "exceptions.h"

// LCOV_EXCL_START
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestModule::DateTime);
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestModule::IsoDate);
BOOST_TEST_DONT_PRINT_LOG_VALUE(DB::Timespan);
// LCOV_EXCL_STOP

class StandardMockDatabase : public PQ::Mock {
	public:
		StandardMockDatabase() : PQ::Mock("user=postgres dbname=postgres", "pqmock", {
				rootDir / "slicer.sql" })
		{
		}
};

BOOST_GLOBAL_FIXTURE( StandardMockDatabase );

typedef boost::shared_ptr<DB::Connection> DBPtr;
typedef boost::shared_ptr<DB::SelectCommand> SelectPtr;

BOOST_AUTO_TEST_CASE( insert_builtins )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	TestModule::BuiltInsPtr bi = new TestModule::BuiltIns(true, 4, 16, 64, 128, 1.2, 3.4, "text");
	Slicer::SerializeAny<Slicer::SqlInsertSerializer>(bi, db.get(), "builtins");
	auto sel = SelectPtr(db->newSelectCommand("SELECT * FROM builtins"));
	auto bi2 = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::BuiltInsPtr>(*sel);
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
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	TestModule::BuiltInSeq bis = {
		TestModule::BuiltInsPtr(new TestModule::BuiltIns(true, 5, 17, 65, 129, 2.3, 4.5, "more text")),
		TestModule::BuiltInsPtr(new TestModule::BuiltIns(true, 6, 18, 66, 130, 3.4, 5.6, "even more text"))
	};
	Slicer::SerializeAny<Slicer::SqlInsertSerializer>(bis, db.get(), "builtins");
	auto sel = SelectPtr(db->newSelectCommand("SELECT * FROM builtins ORDER BY mint"));
	auto bis2 = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::BuiltInSeq>(*sel);
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
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	TestModule::BuiltInSeq bis = {
		TestModule::BuiltInsPtr(new TestModule::BuiltIns(true, 5, 17, 0, 129, 2.3, 4.5, "more text")),
		TestModule::BuiltInsPtr(new TestModule::BuiltIns(true, 6, 18, 0, 130, 3.4, 5.6, "even more text"))
	};
	Slicer::SerializeAny<Slicer::SqlAutoIdInsertSerializer>(bis, db.get(), "builtins");
	auto sel = SelectPtr(db->newSelectCommand("SELECT * FROM builtins WHERE mint IN (1, 2) ORDER BY mint"));
	auto bis2 = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::BuiltInSeq>(*sel);
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
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	TestModule::BuiltInSeq bis = {
		TestModule::BuiltInsPtr(new TestModule::BuiltIns(true, 5, 17, 0, 129, 2.3, 4.5, "more text")),
		TestModule::BuiltInsPtr(new TestModule::BuiltIns(true, 6, 18, 0, 130, 3.4, 5.6, "even more text"))
	};
	Slicer::SerializeAny<Slicer::SqlFetchIdInsertSerializer>(bis, db.get(), "builtins");
	auto sel = SelectPtr(db->newSelectCommand("SELECT * FROM builtins WHERE mint IN (3, 4) ORDER BY mint"));
	auto bis2 = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::BuiltInSeq>(*sel);
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
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	DB::BuiltInSeq bis = {
		DB::BuiltInsPtr(new DB::BuiltIns(true, IceUtil::Optional<Ice::Byte>(), 17, 0, 129, 2.3, 4.5, "more text")),
		DB::BuiltInsPtr(new DB::BuiltIns(true, 6, 18, 0, 130, 3.4, IceUtil::Optional<Ice::Double>(), "even more text"))
	};
	Slicer::SerializeAny<Slicer::SqlFetchIdInsertSerializer>(bis, db.get(), "builtins");
	auto sel = SelectPtr(db->newSelectCommand("SELECT * FROM builtins WHERE mint IN (5, 6) ORDER BY mint"));
	auto bis2 = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, DB::BuiltInSeq>(*sel);
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
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	DB::SpecificTypesPtr st = new DB::SpecificTypes {
		{2015, 10, 16, 19, 12, 34},
		{2015, 10, 16},
		{1, 2, 3, 4}
	};
	Slicer::SerializeAny<Slicer::SqlInsertSerializer>(st, db.get(), "converted");
	auto sel = SelectPtr(db->newSelectCommand("SELECT * FROM converted"));
	auto st2 = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, DB::SpecificTypesPtr>(*sel);
	BOOST_REQUIRE_EQUAL(st->date, st2->date);
	BOOST_REQUIRE_EQUAL(st->dt, st2->dt);
	BOOST_REQUIRE_EQUAL(st->ts, st2->ts);
}

BOOST_AUTO_TEST_CASE( insert_unsupportedModel )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	TestModule::ClassMap cm;
	BOOST_REQUIRE_THROW(Slicer::SerializeAny<Slicer::SqlInsertSerializer>(cm, db.get(), "converted"), Slicer::UnsupportedModelType);
}

