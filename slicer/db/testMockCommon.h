#pragma once

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
