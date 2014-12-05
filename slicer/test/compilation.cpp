#define BOOST_TEST_MODULE compilation
#include <boost/test/unit_test.hpp>

#include <types.h>
#include <slicer/modelParts.h>

namespace std {
	ostream & operator<<(ostream & strm, const type_info & ti)
	{
		strm << ti.name();
		return strm;
	}
}

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
	BOOST_TEST_MESSAGE(typeid(*mpp.get())); \
	BOOST_REQUIRE_EQUAL(typeid(*mpp.get()), typeid(*autoMpp.get())); \
	BOOST_REQUIRE_EQUAL(typeid(*mpp.get()), typeid(*autoPtrMpp.get()));

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
