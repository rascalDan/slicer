#include "sqlSelectDeserializer.h"
#include "testMockCommon.h"
#include <benchmark/benchmark.h>
#include <connection.h>
#include <definedDirs.h>
#include <slicer/slicer.h>
#include <testModels.h>

const StandardMockDatabase db;

class CoreFixture : public benchmark::Fixture, public ConnectionFixture { };

BENCHMARK_F(CoreFixture, bulk_select_complex)(benchmark::State & state)
{
	auto sel = db->select(R"SQL(
			SELECT s mint, CAST(s AS NUMERIC(7,1)) mdouble, CAST(s as text) mstring, s % 2 = 0 mbool
			FROM GENERATE_SERIES(1, 10000) s)SQL");
	for (auto _ : state) {
		benchmark::DoNotOptimize(
				Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, TestDatabase::BuiltInSeq>(sel.get()));
	}
}

BENCHMARK_MAIN();
