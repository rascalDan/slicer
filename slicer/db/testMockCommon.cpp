#include "testMockCommon.h"
#include <definedDirs.h>

StandardMockDatabase::StandardMockDatabase() :
DB::PluginMock<PQ::Mock>("user=postgres dbname=postgres", "pqmock", {
		rootDir.parent_path() / "db" / "slicer.sql" })
{
}

ConnectionFixture::ConnectionFixture() :
	db(DB::MockDatabase::openConnectionTo("pqmock"))
{
}

