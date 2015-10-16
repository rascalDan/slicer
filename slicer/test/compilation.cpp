#define BOOST_TEST_MODULE compilation
#include <boost/test/unit_test.hpp>

#include <types.h>
#include <slicer/modelParts.h>

// LCOV_EXCL_START
BOOST_TEST_DONT_PRINT_LOG_VALUE(std::type_info);
// LCOV_EXCL_STOP

#define TypeTest(Var, Expr, Explicit, Expected) \
	Var obj = Expr; \
	Slicer::ModelPartPtr mpp = new Slicer::Explicit<Var>(obj); \
	BOOST_REQUIRE_EQUAL(Slicer::Expected, mpp->GetType()); \
 \
	Slicer::ModelPartPtr autoMpp = Slicer::ModelPartFor(obj); \
	BOOST_REQUIRE_EQUAL(Slicer::Expected, autoMpp->GetType()); \
 \
	Slicer::ModelPartPtr autoPtrMpp = Slicer::ModelPartFor(&obj); \
	BOOST_REQUIRE_EQUAL(Slicer::Expected, autoPtrMpp->GetType()); \
\
	auto mppvalue = mpp.get(); \
	auto amppvalue = mpp.get(); \
	auto apmppvalue = mpp.get(); \
	BOOST_TEST_CHECKPOINT(typeid(*mppvalue).name()); \
	BOOST_REQUIRE_EQUAL(typeid(*mppvalue), typeid(*amppvalue)); \
	BOOST_REQUIRE_EQUAL(typeid(*mppvalue), typeid(*apmppvalue));

#define StackTypeTest(Var, Explicit, Expected) \
	TypeTest(Var, Var(), Explicit, Expected)

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_class )
{
	TypeTest(TestModule::BuiltInsPtr, new TestModule::BuiltIns(), ModelPartForClass, mpt_Complex);
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_sequenceclasses )
{
	StackTypeTest(TestModule::Classes, ModelPartForSequence, mpt_Sequence);
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_sequencestructs )
{
	StackTypeTest(TestModule::Structs, ModelPartForSequence, mpt_Sequence);
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_mapclasses )
{
	StackTypeTest(TestModule::ClassMap, ModelPartForDictionary, mpt_Dictionary);
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_mapstructs )
{
	StackTypeTest(TestModule::StructMap, ModelPartForDictionary, mpt_Dictionary);
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_bi_string )
{
	StackTypeTest(std::string, ModelPartForSimple, mpt_Simple);
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_bi_bool )
{
	StackTypeTest(bool, ModelPartForSimple, mpt_Simple);
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_bi_float )
{
	StackTypeTest(Ice::Float, ModelPartForSimple, mpt_Simple);
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_bi_double )
{
	StackTypeTest(Ice::Double, ModelPartForSimple, mpt_Simple);
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_bi_byte )
{
	StackTypeTest(Ice::Byte, ModelPartForSimple, mpt_Simple);
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_bi_short )
{
	StackTypeTest(Ice::Short, ModelPartForSimple, mpt_Simple);
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_bi_int )
{
	StackTypeTest(Ice::Int, ModelPartForSimple, mpt_Simple);
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_bi_long )
{
	StackTypeTest(Ice::Long, ModelPartForSimple, mpt_Simple);
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_struct )
{
	StackTypeTest(TestModule::StructType, ModelPartForStruct, mpt_Complex);
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_enum )
{
	StackTypeTest(TestModule::SomeNumbers, ModelPartForEnum, mpt_Simple);
}

