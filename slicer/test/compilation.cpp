#define BOOST_TEST_MODULE compilation
#include <boost/test/unit_test.hpp>

#include <types.h>
#include <locals.h>
#include <slicer/modelParts.h>
#include <slicer/modelPartsTypes.h>

// LCOV_EXCL_START
BOOST_TEST_DONT_PRINT_LOG_VALUE(std::type_info);
// LCOV_EXCL_STOP

namespace pl = std::placeholders;

#define TypeTest(Var, Expr, Explicit, Expected) \
	Var obj = Expr; \
	Slicer::ModelPartPtr mpp = Slicer::ModelPart::CreateFor(obj); \
	BOOST_REQUIRE_EQUAL(Slicer::Expected, mpp->GetType()); \
	\
	BOOST_TEST_CONTEXT(#Var) { \
		auto mppvalue = mpp.get(); \
		auto amppvalue = mpp.get(); \
		auto apmppvalue = mpp.get(); \
		BOOST_TEST_CHECKPOINT(typeid(*mppvalue).name()); \
		BOOST_REQUIRE_EQUAL(typeid(*mppvalue), typeid(*amppvalue)); \
		BOOST_REQUIRE_EQUAL(typeid(*mppvalue), typeid(*apmppvalue)); \
	}

#define StackTypeTest(Var, Explicit, Expected) \
	TypeTest(Var, Var(), Explicit, Expected)

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_class )
{
	TypeTest(TestModule::BuiltInsPtr, std::make_shared<TestModule::BuiltIns>(), ModelPartForClass, mpt_Complex);
	BOOST_REQUIRE_EQUAL(mpp.get(), mpp->GetContainedModelPart().get());
}

static
void
hookHandler(std::vector<std::string> * names, const std::string & name, const Slicer::ModelPartPtr & mpp, const Slicer::HookCommon * h)
{
	names->push_back(name);
	BOOST_REQUIRE(mpp);
	BOOST_REQUIRE(mpp->GetContainedModelPart());
	BOOST_REQUIRE(h);
	BOOST_REQUIRE_EQUAL(h->name, name);
}


BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_sequenceclasses )
{
	StackTypeTest(TestModule::Classes, ModelPartForSequence, mpt_Sequence);
	auto cmpp = mpp->GetContainedModelPart();
	BOOST_REQUIRE(cmpp);
	BOOST_REQUIRE_EQUAL(Slicer::mpt_Complex, cmpp->GetType());
	std::vector<std::string> names;
	cmpp->OnEachChild(std::bind(&hookHandler, &names, pl::_1, pl::_2, pl::_3));
	BOOST_REQUIRE_EQUAL(2, names.size());
	BOOST_REQUIRE_EQUAL("a", names.front());
	BOOST_REQUIRE_EQUAL("b", names.back());
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_sequencestructs )
{
	StackTypeTest(TestModule::Structs, ModelPartForSequence, mpt_Sequence);
	auto cmpp = mpp->GetContainedModelPart();
	BOOST_REQUIRE(cmpp);
	BOOST_REQUIRE_EQUAL(Slicer::mpt_Complex, cmpp->GetType());
	std::vector<std::string> names;
	cmpp->OnEachChild(std::bind(&hookHandler, &names, pl::_1, pl::_2, pl::_3));
	BOOST_REQUIRE_EQUAL(2, names.size());
	BOOST_REQUIRE_EQUAL("a", names.front());
	BOOST_REQUIRE_EQUAL("b", names.back());
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_mapclasses )
{
	StackTypeTest(TestModule::ClassMap, ModelPartForDictionary, mpt_Dictionary);
	auto cmpp = mpp->GetContainedModelPart();
	BOOST_REQUIRE(cmpp);
	BOOST_REQUIRE_EQUAL(Slicer::mpt_Complex, cmpp->GetType());
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_mapstructs )
{
	StackTypeTest(TestModule::StructMap, ModelPartForDictionary, mpt_Dictionary);
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_bi_string )
{
	StackTypeTest(std::string, ModelPartForSimple, mpt_Simple);
	BOOST_REQUIRE_EQUAL(mpp.get(), mpp->GetContainedModelPart().get());
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_bi_bool )
{
	StackTypeTest(bool, ModelPartForSimple, mpt_Simple);
	BOOST_REQUIRE_EQUAL(mpp.get(), mpp->GetContainedModelPart().get());
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_bi_float )
{
	StackTypeTest(Ice::Float, ModelPartForSimple, mpt_Simple);
	BOOST_REQUIRE_EQUAL(mpp.get(), mpp->GetContainedModelPart().get());
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_bi_double )
{
	StackTypeTest(Ice::Double, ModelPartForSimple, mpt_Simple);
	BOOST_REQUIRE_EQUAL(mpp.get(), mpp->GetContainedModelPart().get());
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_bi_byte )
{
	StackTypeTest(Ice::Byte, ModelPartForSimple, mpt_Simple);
	BOOST_REQUIRE_EQUAL(mpp.get(), mpp->GetContainedModelPart().get());
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_bi_short )
{
	StackTypeTest(Ice::Short, ModelPartForSimple, mpt_Simple);
	BOOST_REQUIRE_EQUAL(mpp.get(), mpp->GetContainedModelPart().get());
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_bi_int )
{
	StackTypeTest(Ice::Int, ModelPartForSimple, mpt_Simple);
	BOOST_REQUIRE_EQUAL(mpp.get(), mpp->GetContainedModelPart().get());
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_bi_long )
{
	StackTypeTest(Ice::Long, ModelPartForSimple, mpt_Simple);
	BOOST_REQUIRE_EQUAL(mpp.get(), mpp->GetContainedModelPart().get());
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_struct )
{
	StackTypeTest(TestModule::StructType, ModelPartForStruct, mpt_Complex);
	BOOST_REQUIRE_EQUAL(mpp.get(), mpp->GetContainedModelPart().get());
}

BOOST_AUTO_TEST_CASE( compile_auto_modelpart_type_enum )
{
	StackTypeTest(TestModule::SomeNumbers, ModelPartForEnum, mpt_Simple);
	BOOST_REQUIRE_EQUAL(mpp.get(), mpp->GetContainedModelPart().get());
}

BOOST_AUTO_TEST_CASE( normalClassTypeId )
{
	TestModule::BasePtr base = std::make_shared<TestModule::Base>(1);
	BOOST_REQUIRE(base);
	auto a = Slicer::ModelPart::CreateFor(base);
	BOOST_REQUIRE(a);
	auto baseType = a->GetTypeId();
	BOOST_REQUIRE(!baseType);
}

BOOST_AUTO_TEST_CASE( normalSubClassTypeId )
{
	TestModule::BasePtr base = std::make_shared<TestModule::D1>(1, 2);
	BOOST_REQUIRE(base);
	auto a = Slicer::ModelPart::CreateFor(base);
	BOOST_REQUIRE(a);
	auto baseType = a->GetTypeId();
	BOOST_REQUIRE(baseType);
	BOOST_REQUIRE_EQUAL(*baseType, "::TestModule::D1");
}

BOOST_AUTO_TEST_CASE( normalSubSubClassTypeId )
{
	TestModule::BasePtr base = std::make_shared<TestModule::D3>(1, 2, 3);
	BOOST_REQUIRE(base);
	auto a = Slicer::ModelPart::CreateFor(base);
	BOOST_REQUIRE(a);
	auto baseType = a->GetTypeId();
	BOOST_REQUIRE(baseType);
	BOOST_REQUIRE_EQUAL(*baseType, "::TestModule::D3");
}

BOOST_AUTO_TEST_CASE( localClassTypeId )
{
	Locals::LocalClassPtr base = std::make_shared<Locals::LocalClass>(1, "One");
	BOOST_REQUIRE(base);
	auto a = Slicer::ModelPart::CreateFor(base);
	BOOST_REQUIRE(a);
	auto baseType = a->GetTypeId();
	BOOST_REQUIRE(!baseType);
}

BOOST_AUTO_TEST_CASE( localSubClassTypeId )
{
	Locals::LocalClassPtr base = std::make_shared<Locals::LocalSubClass>(1, "One", 3.1);
	BOOST_REQUIRE(base);
	auto a = Slicer::ModelPart::CreateFor(base);
	BOOST_REQUIRE(a);
	auto baseType = a->GetTypeId();
	BOOST_REQUIRE(baseType);
	BOOST_REQUIRE_EQUAL(*baseType, "::Locals::LocalSubClass");
}

BOOST_AUTO_TEST_CASE( localSubSubClassTypeId )
{
	Locals::LocalClassPtr base = std::make_shared<Locals::LocalSub2Class>(1, "One", 3.1, 1);
	BOOST_REQUIRE(base);
	auto a = Slicer::ModelPart::CreateFor(base);
	BOOST_REQUIRE(a);
	auto baseType = a->GetTypeId();
	BOOST_REQUIRE(baseType);
	BOOST_REQUIRE_EQUAL(*baseType, "::Locals::LocalSub2Class");
}

