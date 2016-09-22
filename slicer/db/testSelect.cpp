#define BOOST_TEST_MODULE db_select
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <pq-mock.h>
#include <slicer/slicer.h>
#include <definedDirs.h>
#include "sqlSelectDeserializer.h"
#include <types.h>
#include <common.h>
#include <sqlExceptions.h>

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

BOOST_AUTO_TEST_CASE( select_simple_int )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = SelectPtr(db->newSelectCommand("SELECT MAX(id) FROM test"));
	auto bi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, Ice::Int>(*sel);
	BOOST_REQUIRE_EQUAL(4, bi);
}

BOOST_AUTO_TEST_CASE( select_simple_double )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = SelectPtr(db->newSelectCommand("SELECT MAX(fl) FROM test"));
	auto bi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, Ice::Double>(*sel);
	BOOST_REQUIRE_CLOSE(1234.1234, bi, 0.0001);
}

BOOST_AUTO_TEST_CASE( select_simple_string )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = SelectPtr(db->newSelectCommand("SELECT MAX(string) FROM test"));
	auto bi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, std::string>(*sel);
	BOOST_REQUIRE_EQUAL("text two", bi);
}

BOOST_AUTO_TEST_CASE( select_simple_true )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = SelectPtr(db->newSelectCommand("SELECT true"));
	auto bi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, bool>(*sel);
	BOOST_REQUIRE_EQUAL(true, bi);
}

BOOST_AUTO_TEST_CASE( select_simple_false )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = SelectPtr(db->newSelectCommand("SELECT NOT(true)"));
	auto bi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, bool>(*sel);
	BOOST_REQUIRE_EQUAL(false, bi);
}

BOOST_AUTO_TEST_CASE( select_single )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = SelectPtr(db->newSelectCommand(
				"SELECT boolean mbool, \
				id mbyte, id mshort, id mint, id mlong, \
				fl mdouble, fl mfloat, \
				string mstring \
				FROM test \
				ORDER BY id \
				LIMIT 1"));
	auto bi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::BuiltInsPtr>(*sel);
	BOOST_REQUIRE(bi);
	BOOST_REQUIRE_EQUAL(true, bi->mbool);
	BOOST_REQUIRE_EQUAL(1, bi->mbyte);
	BOOST_REQUIRE_EQUAL(1, bi->mshort);
	BOOST_REQUIRE_EQUAL(1, bi->mint);
	BOOST_REQUIRE_EQUAL(1, bi->mlong);
	BOOST_REQUIRE_CLOSE(1.1, bi->mfloat, 0.0001);
	BOOST_REQUIRE_CLOSE(1.1, bi->mdouble, 0.0001);
	BOOST_REQUIRE_EQUAL("text one", bi->mstring);
}

BOOST_AUTO_TEST_CASE( select_inherit_single )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = SelectPtr(db->newSelectCommand(
				"SELECT id a, '::TestModule::D' || CAST(id AS TEXT) tc, 200 b, 300 c, 400 d \
				FROM test \
				WHERE id = 2"));
	auto bi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::BasePtr>(*sel, "tc");
	BOOST_REQUIRE(bi);
	auto d2 = TestModule::D2Ptr::dynamicCast(bi);
	BOOST_REQUIRE(d2);
	BOOST_REQUIRE_EQUAL(2, d2->a);
	BOOST_REQUIRE_EQUAL(300, d2->c);
}

BOOST_AUTO_TEST_CASE( select_simple_sequence )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = SelectPtr(db->newSelectCommand(
				"SELECT string \
				FROM test \
				ORDER BY id DESC"));
	auto bi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::SimpleSeq>(*sel);
	BOOST_REQUIRE_EQUAL(4, bi.size());
	BOOST_REQUIRE_EQUAL("text four", bi[0]);
	BOOST_REQUIRE_EQUAL("text three", bi[1]);
	BOOST_REQUIRE_EQUAL("text two", bi[2]);
	BOOST_REQUIRE_EQUAL("text one", bi[3]);
}

BOOST_AUTO_TEST_CASE( select_inherit_sequence )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = SelectPtr(db->newSelectCommand(
				"SELECT id a, '::TestModule::D' || CAST(id AS TEXT) tc, 200 b, 300 c, 400 d \
				FROM test \
				WHERE id < 4 \
				ORDER BY id DESC"));
	auto bi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::BaseSeq>(*sel, "tc");
	BOOST_REQUIRE_EQUAL(3, bi.size());
	auto d3 = TestModule::D3Ptr::dynamicCast(bi[0]);
	auto d2 = TestModule::D2Ptr::dynamicCast(bi[1]);
	auto d1 = TestModule::D1Ptr::dynamicCast(bi[2]);
	BOOST_REQUIRE(d3);
	BOOST_REQUIRE(d2);
	BOOST_REQUIRE(d1);
	BOOST_REQUIRE_EQUAL(3, d3->a);
	BOOST_REQUIRE_EQUAL(300, d3->c);
	BOOST_REQUIRE_EQUAL(400, d3->d);
	BOOST_REQUIRE_EQUAL(2, d2->a);
	BOOST_REQUIRE_EQUAL(300, d2->c);
	BOOST_REQUIRE_EQUAL(1, d1->a);
	BOOST_REQUIRE_EQUAL(200, d1->b);
}

BOOST_AUTO_TEST_CASE( select_inherit_datetime )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = SelectPtr(db->newSelectCommand(
				"SELECT dt, to_char(dt, 'YYYY-MM-DD') date, ts \
				FROM test \
				WHERE id = 3"));
	DB::SpecificTypesPtr bi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, DB::SpecificTypesPtr>(*sel);
	BOOST_REQUIRE_EQUAL(2015, bi->dt.year);
	BOOST_REQUIRE_EQUAL(3, bi->dt.month);
	BOOST_REQUIRE_EQUAL(27, bi->dt.day);
	BOOST_REQUIRE_EQUAL(23, bi->dt.hour);
	BOOST_REQUIRE_EQUAL(6, bi->dt.minute);
	BOOST_REQUIRE_EQUAL(3, bi->dt.second);
	BOOST_REQUIRE_EQUAL(2015, bi->date.year);
	BOOST_REQUIRE_EQUAL(3, bi->date.month);
	BOOST_REQUIRE_EQUAL(27, bi->date.day);
	BOOST_REQUIRE_EQUAL(1, bi->ts->days);
	BOOST_REQUIRE_EQUAL(13, bi->ts->hours);
	BOOST_REQUIRE_EQUAL(13, bi->ts->minutes);
	BOOST_REQUIRE_EQUAL(12, bi->ts->seconds);
}

template <typename T, typename ... P>
T
BoostThrowWrapperHelper(P && ... p)
{
	return Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, T>(p...);
}

BOOST_AUTO_TEST_CASE( select_unsupportedModel )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = SelectPtr(db->newSelectCommand("SELECT id FROM test"));
	BOOST_REQUIRE_THROW(BoostThrowWrapperHelper<TestModule::ClassMap>(*sel), Slicer::UnsupportedModelType);
}

BOOST_AUTO_TEST_CASE( select_tooManyRowsSimple )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = SelectPtr(db->newSelectCommand("SELECT id FROM test"));
	BOOST_REQUIRE_THROW(BoostThrowWrapperHelper<Ice::Int>(*sel), Slicer::TooManyRowsReturned);
}

BOOST_AUTO_TEST_CASE( select_noRowsSimple )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = SelectPtr(db->newSelectCommand("SELECT id FROM test WHERE false"));
	BOOST_REQUIRE_THROW(BoostThrowWrapperHelper<Ice::Int>(*sel), Slicer::NoRowsReturned);
}

BOOST_AUTO_TEST_CASE( select_noRowsSimpleOptional )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = SelectPtr(db->newSelectCommand("SELECT id FROM test WHERE false"));
	auto v = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, IceUtil::Optional<Ice::Int>>(*sel);
	BOOST_REQUIRE(!v);
}

BOOST_AUTO_TEST_CASE( select_tooManyRowsSimpleOptional )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = SelectPtr(db->newSelectCommand("SELECT id FROM test"));
	BOOST_REQUIRE_THROW(BoostThrowWrapperHelper<IceUtil::Optional<Ice::Int>>(*sel), Slicer::TooManyRowsReturned);
}

BOOST_AUTO_TEST_CASE( select_simpleOptional )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = SelectPtr(db->newSelectCommand("SELECT MAX(id) FROM test"));
	auto v = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, IceUtil::Optional<Ice::Int>>(*sel);
	BOOST_REQUIRE(v);
	BOOST_REQUIRE_EQUAL(4, *v);
}

BOOST_AUTO_TEST_CASE( select_noRowsComplexOptional )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = SelectPtr(db->newSelectCommand(
				"SELECT boolean mbool, \
				id mbyte, id mshort, id mint, id mlong, \
				fl mdouble, fl mfloat, \
				string mstring \
				FROM test \
				WHERE false"));
	auto v = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, IceUtil::Optional<TestModule::BuiltInsPtr>>(*sel);
	BOOST_REQUIRE(!v);
}

BOOST_AUTO_TEST_CASE( select_tooManyRowsComplex )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = SelectPtr(db->newSelectCommand("SELECT id FROM test"));
	BOOST_REQUIRE_THROW(BoostThrowWrapperHelper<TestModule::BuiltInsPtr>(*sel), Slicer::TooManyRowsReturned);
}

BOOST_AUTO_TEST_CASE( select_noRowsComplex )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = SelectPtr(db->newSelectCommand("SELECT id FROM test WHERE false"));
	BOOST_REQUIRE_THROW(BoostThrowWrapperHelper<TestModule::BuiltInsPtr>(*sel), Slicer::NoRowsReturned);
}

BOOST_AUTO_TEST_CASE( select_emptySequence )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = SelectPtr(db->newSelectCommand("SELECT id FROM test WHERE false"));
	auto bi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::BaseSeq>(*sel);
	BOOST_REQUIRE_EQUAL(0, bi.size());
}

BOOST_AUTO_TEST_CASE( select_null )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	db->execute("INSERT INTO test(id) VALUES(NULL)");

	auto sel = SelectPtr(db->newSelectCommand("SELECT id optSimple FROM test WHERE id IS NULL"));
	auto oi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::OptionalsPtr>(*sel);
	BOOST_REQUIRE(!oi->optSimple);

	sel = SelectPtr(db->newSelectCommand("SELECT MAX(id) optSimple FROM test WHERE id IS NOT NULL"));
	oi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::OptionalsPtr>(*sel);
	BOOST_REQUIRE(oi->optSimple);
	BOOST_REQUIRE_EQUAL(oi->optSimple.get(), 4);

	sel = SelectPtr(db->newSelectCommand("SELECT MAX(id) FROM test WHERE false"));
	auto v = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, IceUtil::Optional<Ice::Int>>(*sel);
	BOOST_REQUIRE(!v);
}

