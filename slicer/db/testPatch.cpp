#define BOOST_TEST_MODULE db_patch
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <pq-mock.h>
#include <slicer/slicer.h>
#include <definedDirs.h>
#include "sqlTablePatchSerializer.h"
#include "sqlSelectDeserializer.h"
#include <types.h>
#include <common.h>
#include <db.h>

// LCOV_EXCL_START
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestModule::DateTime);
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestModule::IsoDate);
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestDatabase::Timespan);
// LCOV_EXCL_STOP

class StandardMockDatabase : public PQ::Mock {
	public:
		StandardMockDatabase() : PQ::Mock("user=postgres dbname=postgres", "pqmock", {
				rootDir.parent_path() / "db" / "slicer.sql" })
		{
		}
};

BOOST_GLOBAL_FIXTURE( StandardMockDatabase );

typedef boost::shared_ptr<DB::Connection> DBPtr;
typedef boost::shared_ptr<DB::SelectCommand> SelectPtr;

BOOST_AUTO_TEST_CASE( insert_builtins )
{
	auto db = DBPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	TestModule::BuiltInSeq bis = {
		TestModule::BuiltInsPtr(new TestModule::BuiltIns(true, 5, 17, 0, 129, 2.3, 4.5, "more text")),
		TestModule::BuiltInsPtr(new TestModule::BuiltIns(true, 6, 18, 0, 130, 3.4, 5.6, "even more text"))
	};
	DB::TablePatch tp;
	DB::TransactionScope tx(db.get());
	tp.dest = "builtins";
	Slicer::SerializeAny<Slicer::SqlTablePatchSerializer>(bis, db.get(), tp);
	auto cmd = db->select("SELECT COUNT(*) FROM builtins");
	auto c = Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, int>(*cmd);
	BOOST_REQUIRE_EQUAL(2, c);
}

