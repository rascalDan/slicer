#define BOOST_TEST_MODULE db_select
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include "testMockCommon.h"
#include <slicer/slicer.h>
#include "sqlSelectDeserializer.h"
#include <types.h>
#include <common.h>
#include <testModels.h>
#include <sqlExceptions.h>

using namespace std::literals;

BOOST_GLOBAL_FIXTURE( StandardMockDatabase );

BOOST_FIXTURE_TEST_SUITE(db, ConnectionFixture);

BOOST_AUTO_TEST_CASE( select_simple_int )
{
	auto sel = db->select("SELECT MAX(id) FROM test");
	auto bi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, Ice::Int>(sel.get());
	BOOST_REQUIRE_EQUAL(4, bi);
}

BOOST_AUTO_TEST_CASE( select_simple_double )
{
	auto sel = db->select("SELECT MAX(fl) FROM test");
	auto bi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, Ice::Double>(sel.get());
	BOOST_REQUIRE_CLOSE(1234.1234, bi, 0.0001);
}

BOOST_AUTO_TEST_CASE( select_simple_string )
{
	auto sel = db->select("SELECT MAX(string) FROM test");
	auto bi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, std::string>(sel.get());
	BOOST_REQUIRE_EQUAL("text two", bi);
}

BOOST_AUTO_TEST_CASE( select_simple_true )
{
	auto sel = db->select("SELECT true");
	auto bi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, bool>(sel.get());
	BOOST_REQUIRE_EQUAL(true, bi);
}

BOOST_AUTO_TEST_CASE( select_simple_false )
{
	auto sel = db->select("SELECT NOT(true)");
	auto bi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, bool>(sel.get());
	BOOST_REQUIRE_EQUAL(false, bi);
}

BOOST_AUTO_TEST_CASE( select_single )
{
	auto sel = db->select(
				"SELECT boolean mbool, \
				id mbyte, id mshort, id mint, id mlong, \
				fl mdouble, fl mfloat, \
				string mstring \
				FROM test \
				ORDER BY id \
				LIMIT 1");
	auto bi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::BuiltInsPtr>(sel.get());
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
	auto sel = db->select(
				"SELECT id a, '::TestModule::D' || CAST(id AS TEXT) tc, 200 b, 300 c, 400 d \
				FROM test \
				WHERE id = 2");
	auto bi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::BasePtr>(sel.get(), "tc"s);
	BOOST_REQUIRE(bi);
	auto d2 = std::dynamic_pointer_cast<TestModule::D2>(bi);
	BOOST_REQUIRE(d2);
	BOOST_REQUIRE_EQUAL(2, d2->a);
	BOOST_REQUIRE_EQUAL(300, d2->c);
}

BOOST_AUTO_TEST_CASE( select_simple_sequence )
{
	auto sel = db->select(
				"SELECT string \
				FROM test \
				ORDER BY id DESC");
	auto bi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::SimpleSeq>(sel.get());
	BOOST_REQUIRE_EQUAL(4, bi.size());
	BOOST_REQUIRE_EQUAL("text four", bi[0]);
	BOOST_REQUIRE_EQUAL("text three", bi[1]);
	BOOST_REQUIRE_EQUAL("text two", bi[2]);
	BOOST_REQUIRE_EQUAL("text one", bi[3]);
}

BOOST_AUTO_TEST_CASE( select_inherit_sequence )
{
	auto sel = db->select(
				"SELECT id a, '::TestModule::D' || CAST(id AS TEXT) tc, 200 b, 300 c, 400 d \
				FROM test \
				WHERE id < 4 \
				ORDER BY id DESC");
	auto bi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::BaseSeq>(sel.get(), "tc"s);
	BOOST_REQUIRE_EQUAL(3, bi.size());
	auto d3 = std::dynamic_pointer_cast<TestModule::D3>(bi[0]);
	auto d2 = std::dynamic_pointer_cast<TestModule::D2>(bi[1]);
	auto d1 = std::dynamic_pointer_cast<TestModule::D1>(bi[2]);
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
	auto sel = db->select(
				"SELECT dt, to_char(dt, 'YYYY-MM-DD') date, ts \
				FROM test \
				WHERE id = 3");
	TestDatabase::SpecificTypesPtr bi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestDatabase::SpecificTypesPtr>(sel.get());
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
	auto sel = db->select("SELECT id FROM test");
	BOOST_REQUIRE_THROW(BoostThrowWrapperHelper<TestModule::ClassMap>(sel.get()), Slicer::UnsupportedModelType);
}

BOOST_AUTO_TEST_CASE( select_tooManyRowsSimple )
{
	auto sel = db->select("SELECT id FROM test");
	BOOST_REQUIRE_THROW(BoostThrowWrapperHelper<Ice::Int>(sel.get()), Slicer::TooManyRowsReturned);
}

BOOST_AUTO_TEST_CASE( select_noRowsSimple )
{
	auto sel = db->select("SELECT id FROM test WHERE false");
	BOOST_REQUIRE_THROW(BoostThrowWrapperHelper<Ice::Int>(sel.get()), Slicer::NoRowsReturned);
}

BOOST_AUTO_TEST_CASE( select_noRowsSimpleOptional )
{
	auto sel = db->select("SELECT id FROM test WHERE false");
	auto v = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, Ice::optional<Ice::Int>>(sel.get());
	BOOST_REQUIRE(!v);
}

BOOST_AUTO_TEST_CASE( select_tooManyRowsSimpleOptional )
{
	auto sel = db->select("SELECT id FROM test");
	BOOST_REQUIRE_THROW(BoostThrowWrapperHelper<Ice::optional<Ice::Int>>(sel.get()), Slicer::TooManyRowsReturned);
}

BOOST_AUTO_TEST_CASE( select_simpleOptional )
{
	auto sel = db->select("SELECT MAX(id) FROM test");
	auto v = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, Ice::optional<Ice::Int>>(sel.get());
	BOOST_REQUIRE(v);
	BOOST_REQUIRE_EQUAL(4, *v);
}

BOOST_AUTO_TEST_CASE( select_noRowsComplexOptional )
{
	auto sel = db->select(
				"SELECT boolean mbool, \
				id mbyte, id mshort, id mint, id mlong, \
				fl mdouble, fl mfloat, \
				string mstring \
				FROM test \
				WHERE false");
	auto v = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, Ice::optional<TestModule::BuiltInsPtr>>(sel.get());
	BOOST_REQUIRE(!v);
}

BOOST_AUTO_TEST_CASE( select_tooManyRowsComplex )
{
	auto sel = db->select("SELECT id FROM test");
	BOOST_REQUIRE_THROW(BoostThrowWrapperHelper<TestModule::BuiltInsPtr>(sel.get()), Slicer::TooManyRowsReturned);
}

BOOST_AUTO_TEST_CASE( select_noRowsComplex )
{
	auto sel = db->select("SELECT id FROM test WHERE false");
	BOOST_REQUIRE_THROW(BoostThrowWrapperHelper<TestModule::BuiltInsPtr>(sel.get()), Slicer::NoRowsReturned);
}

BOOST_AUTO_TEST_CASE( select_emptySequence )
{
	auto sel = db->select("SELECT id FROM test WHERE false");
	auto bi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::BaseSeq>(sel.get());
	BOOST_REQUIRE_EQUAL(0, bi.size());
}

BOOST_AUTO_TEST_CASE( select_null )
{
	db->execute("INSERT INTO test(id) VALUES(NULL)");

	auto sel = db->select("SELECT id optSimple FROM test WHERE id IS NULL");
	auto oi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::OptionalsPtr>(sel.get());
	BOOST_REQUIRE(!oi->optSimple);

	sel = db->select("SELECT MAX(id) optSimple FROM test WHERE id IS NOT NULL");
	oi = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestModule::OptionalsPtr>(sel.get());
	BOOST_REQUIRE(oi->optSimple);
	BOOST_REQUIRE_EQUAL(*oi->optSimple, 4);

	sel = db->select("SELECT MAX(id) FROM test WHERE false");
	auto v = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, Ice::optional<Ice::Int>>(sel.get());
	BOOST_REQUIRE(!v);
}

BOOST_AUTO_TEST_CASE( bulkSelectTest )
{
	auto sel = db->select(R"SQL(select s mint, cast(s as numeric(7,1)) mdouble, cast(s as text) mstring, s % 2 = 0 mbool from generate_series(1, 10000) s)SQL");
	auto vec = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestDatabase::BuiltInSeq>(sel.get());
	BOOST_REQUIRE_EQUAL(10000, vec.size());
}

BOOST_AUTO_TEST_SUITE_END();

