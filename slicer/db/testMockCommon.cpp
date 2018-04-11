#include "testMockCommon.h"
#include <definedDirs.h>

StandardMockDatabase::StandardMockDatabase() :
DB::PluginMock<PQ::Mock>("user=postgres dbname=postgres", "pqmock", {
		rootDir.parent_path() / "db" / "slicer.sql" })
{
}

ConnectionFixture::ConnectionFixture() :
	_db(DB::MockDatabase::openConnectionTo("pqmock")),
	db(_db.get())
{
}

