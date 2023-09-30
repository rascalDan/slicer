#include "testMockCommon.h"
#include <connection_fwd.h>
#include <definedDirs.h>
#include <filesystem>
#include <mockDatabase.h>
#include <pq-mock.h>

StandardMockDatabase::StandardMockDatabase() :
	DB::PluginMock<PQ::Mock>("pqmock", {rootDir.parent_path() / "db" / "slicer.sql"}, "user=postgres dbname=postgres")
{
}

ConnectionFixture::ConnectionFixture() : _db(DB::MockDatabase::openConnectionTo("pqmock")), db(_db.get()) { }
