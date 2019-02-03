#define BOOST_TEST_MODULE db_patch
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include "testMockCommon.h"
#include <slicer/slicer.h>
#include "sqlTablePatchSerializer.h"
#include "sqlSelectDeserializer.h"
#include <types.h>
#include <common.h>
#include <testModels.h>

// LCOV_EXCL_START
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestModule::DateTime);
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestModule::IsoDate);
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestDatabase::Timespan);
BOOST_TEST_DONT_PRINT_LOG_VALUE(DB::PrimaryKey);
// LCOV_EXCL_STOP

BOOST_GLOBAL_FIXTURE( StandardMockDatabase );

BOOST_FIXTURE_TEST_SUITE(db, ConnectionFixture);

BOOST_AUTO_TEST_CASE( insert_builtins )
{
	TestModule::BuiltInSeq bis = {
		std::make_shared<TestModule::BuiltIns>(true, 5, 17, 0, 129, 2.3, 4.5, "more text"),
		std::make_shared<TestModule::BuiltIns>(true, 6, 18, 0, 130, 3.4, 5.6, "even more text")
	};
	DB::TablePatch tp;
	DB::TransactionScope tx(*db);
	tp.dest = "builtins";
	Slicer::SerializeAny<Slicer::SqlTablePatchSerializer>(bis, db, tp);
	auto cmd = db->select("SELECT COUNT(*) FROM builtins");
	auto c = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, int>(cmd.get());
	BOOST_REQUIRE_EQUAL(2, c);
	BOOST_REQUIRE_EQUAL(2, tp.pk.size());
	DB::PrimaryKey pk = {"mint", "mlong"};
	BOOST_REQUIRE_EQUAL(pk, tp.pk);
	BOOST_REQUIRE_EQUAL(8, tp.cols.size());
	DB::ColumnNames cols = {"mbool", "mbyte", "mdouble", "mfloat", "mint", "mlong", "mshort", "mstring"};
	BOOST_REQUIRE_EQUAL(cols, tp.cols);
}

BOOST_AUTO_TEST_SUITE_END();

