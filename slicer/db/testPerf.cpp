#include "sqlSelectDeserializer.h"
#include "testMockCommon.h"
#include <benchmark/benchmark.h>
#include <collections.h>
#include <connection.h>
#include <definedDirs.h>
#include <slicer/slicer.h>
#include <testModels.h>

const StandardMockDatabase db;

class CoreFixture : public benchmark::Fixture, public ConnectionFixture {
protected:
	template<typename Out>
	void
	do_bulk_select_complex(benchmark::State & state)
	{
		auto sel = db->select(R"SQL(
			SELECT s mint, CAST(s AS NUMERIC(7,1)) mdouble, CAST(s as text) mstring, s % 2 = 0 mbool
			FROM GENERATE_SERIES(1, 10000) s)SQL");
		for (auto _ : state) {
			benchmark::DoNotOptimize(Slicer::DeserializeAny<Slicer::SqlSelectDeserializer, Out>(sel.get()));
		}
	}
};

BENCHMARK_F(CoreFixture, bulk_select_complex)(benchmark::State & state)
{
	do_bulk_select_complex<TestDatabase::BuiltInSeq>(state);
}

BENCHMARK_F(CoreFixture, bulk_select_complex_non_optional)(benchmark::State & state)
{
	do_bulk_select_complex<TestModule::BuiltInSeq>(state);
}

BENCHMARK_MAIN();
