#ifndef SLICER_DB_MOCKDB_H
#define SLICER_DB_MOCKDB_H

#include <connection_fwd.h>
#include <mockDatabase.h>
#include <pq-mock.h>
#include <visibility.h>

// IWYU pragma: no_forward_declare PQ::Mock
namespace DB {
	class Connection;
}

class DLL_PUBLIC StandardMockDatabase : public DB::PluginMock<PQ::Mock> {
public:
	StandardMockDatabase();
};

class DLL_PUBLIC ConnectionFixture {
public:
	ConnectionFixture();

	DB::ConnectionPtr _db;
	DB::Connection * const db;
};

#endif
