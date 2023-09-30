#define BOOST_TEST_MODULE compilation
#include <boost/test/unit_test.hpp>

#include "classes.h"
#include "classtype.h"
#include "collections.h"
#include "enums.h"
#include "inheritance.h"
#include "locals.h"
#include "slicer/modelParts.h"
#include "structs.h"
#include <Ice/Config.h>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

// LCOV_EXCL_START
// cppcheck-suppress unknownMacro
BOOST_TEST_DONT_PRINT_LOG_VALUE(std::type_info)
// cppcheck-suppress unknownMacro
BOOST_TEST_DONT_PRINT_LOG_VALUE(Slicer::ModelPartType)
// LCOV_EXCL_STOP

#define TypeTest(Var, Expr, Explicit, Expected, ...) \
	Var obj = Expr; \
	Slicer::ModelPart::CreateFor(&obj, [](auto && mpp) { \
		BOOST_REQUIRE_EQUAL(Slicer::Expected, mpp->GetType()); \
\
		BOOST_TEST_CONTEXT(#Var) { \
			auto mppvalue = mpp.get(); \
			auto amppvalue = mpp.get(); \
			auto apmppvalue = mpp.get(); \
			BOOST_TEST_CONTEXT(typeid(*mppvalue).name()) { \
				BOOST_REQUIRE_EQUAL(typeid(*mppvalue), typeid(*amppvalue)); \
				BOOST_REQUIRE_EQUAL(typeid(*mppvalue), typeid(*apmppvalue)); \
			} \
		} \
		__VA_ARGS__ \
	});

#define StackTypeTest(Var, Explicit, Expected, ...) TypeTest(Var, Var(), Explicit, Expected, __VA_ARGS__)

constexpr auto DontCall = [](auto &&...) {
	BOOST_ERROR("Shouldn't be called");
};

BOOST_AUTO_TEST_CASE(compile_auto_modelpart_type_class)
{
	TypeTest(TestModule::BuiltInsPtr, std::make_shared<TestModule::BuiltIns>(), ModelPartForClass,
			ModelPartType::Complex, { BOOST_CHECK_THROW(mpp->OnContained(DontCall), std::logic_error); });
}

namespace {
	void
	hookHandler(std::vector<std::string> * names, const std::string & name, Slicer::ModelPartParam mpp,
			const Slicer::HookCommon * h)
	{
		names->push_back(name);
		BOOST_REQUIRE(mpp);
		BOOST_CHECK_THROW(mpp->OnContained(DontCall), std::logic_error);
		BOOST_REQUIRE(h);
		BOOST_REQUIRE_EQUAL(h->name, name);
	}
}

BOOST_AUTO_TEST_CASE(compile_auto_modelpart_type_sequenceclasses)
{
	StackTypeTest(TestModule::Classes, ModelPartForSequence, ModelPartType::Sequence, {
		mpp->OnContained([](auto && cmpp) {
			BOOST_REQUIRE(cmpp);
			BOOST_REQUIRE_EQUAL(Slicer::ModelPartType::Complex, cmpp->GetType());
			std::vector<std::string> names;
			cmpp->OnEachChild([&](auto && PH1, auto && PH2, auto && PH3) {
				hookHandler(&names, PH1, PH2, PH3);
			});
			BOOST_REQUIRE_EQUAL(2, names.size());
			BOOST_REQUIRE_EQUAL("a", names.front());
			BOOST_REQUIRE_EQUAL("b", names.back());
		});
	});
}

BOOST_AUTO_TEST_CASE(compile_auto_modelpart_type_sequencestructs)
{
	StackTypeTest(TestModule::Structs, ModelPartForSequence, ModelPartType::Sequence, {
		mpp->OnContained([](auto && cmpp) {
			BOOST_REQUIRE(cmpp);
			BOOST_REQUIRE_EQUAL(Slicer::ModelPartType::Complex, cmpp->GetType());
			std::vector<std::string> names;
			cmpp->OnEachChild([&](auto && PH1, auto && PH2, auto && PH3) {
				hookHandler(&names, PH1, PH2, PH3);
			});
			BOOST_REQUIRE_EQUAL(2, names.size());
			BOOST_REQUIRE_EQUAL("a", names.front());
			BOOST_REQUIRE_EQUAL("b", names.back());
		});
	});
}

BOOST_AUTO_TEST_CASE(compile_auto_modelpart_type_mapclasses)
{
	StackTypeTest(TestModule::ClassMap, ModelPartForDictionary, ModelPartType::Dictionary, {
		mpp->OnContained([](auto && cmpp) {
			BOOST_REQUIRE(cmpp);
			BOOST_REQUIRE_EQUAL(Slicer::ModelPartType::Complex, cmpp->GetType());
		});
	});
}

BOOST_AUTO_TEST_CASE(compile_auto_modelpart_type_mapstructs)
{
	StackTypeTest(TestModule::StructMap, ModelPartForDictionary, ModelPartType::Dictionary);
}

BOOST_AUTO_TEST_CASE(compile_auto_modelpart_type_bi_string)
{
	StackTypeTest(std::string, ModelPartForSimple, ModelPartType::Simple,
			{ BOOST_CHECK_THROW(mpp->OnContained(DontCall), std::logic_error); });
}

BOOST_AUTO_TEST_CASE(compile_auto_modelpart_type_bi_bool)
{
	StackTypeTest(bool, ModelPartForSimple, ModelPartType::Simple,
			{ BOOST_CHECK_THROW(mpp->OnContained(DontCall), std::logic_error); });
}

BOOST_AUTO_TEST_CASE(compile_auto_modelpart_type_bi_float)
{
	StackTypeTest(Ice::Float, ModelPartForSimple, ModelPartType::Simple,
			{ BOOST_CHECK_THROW(mpp->OnContained(DontCall), std::logic_error); });
}

BOOST_AUTO_TEST_CASE(compile_auto_modelpart_type_bi_double)
{
	StackTypeTest(Ice::Double, ModelPartForSimple, ModelPartType::Simple,
			{ BOOST_CHECK_THROW(mpp->OnContained(DontCall), std::logic_error); });
}

BOOST_AUTO_TEST_CASE(compile_auto_modelpart_type_bi_byte)
{
	StackTypeTest(Ice::Byte, ModelPartForSimple, ModelPartType::Simple,
			{ BOOST_CHECK_THROW(mpp->OnContained(DontCall), std::logic_error); });
}

BOOST_AUTO_TEST_CASE(compile_auto_modelpart_type_bi_short)
{
	StackTypeTest(Ice::Short, ModelPartForSimple, ModelPartType::Simple,
			{ BOOST_CHECK_THROW(mpp->OnContained(DontCall), std::logic_error); });
}

BOOST_AUTO_TEST_CASE(compile_auto_modelpart_type_bi_int)
{
	StackTypeTest(Ice::Int, ModelPartForSimple, ModelPartType::Simple,
			{ BOOST_CHECK_THROW(mpp->OnContained(DontCall), std::logic_error); });
}

BOOST_AUTO_TEST_CASE(compile_auto_modelpart_type_bi_long)
{
	StackTypeTest(Ice::Long, ModelPartForSimple, ModelPartType::Simple,
			{ BOOST_CHECK_THROW(mpp->OnContained(DontCall), std::logic_error); });
}

BOOST_AUTO_TEST_CASE(compile_auto_modelpart_type_struct)
{
	StackTypeTest(TestModule::StructType, ModelPartForStruct, ModelPartType::Complex,
			{ BOOST_CHECK_THROW(mpp->OnContained(DontCall), std::logic_error); });
}

BOOST_AUTO_TEST_CASE(compile_auto_modelpart_type_enum)
{
	StackTypeTest(TestModule::SomeNumbers, ModelPartForEnum, ModelPartType::Simple,
			{ BOOST_CHECK_THROW(mpp->OnContained(DontCall), std::logic_error); });
}

BOOST_AUTO_TEST_CASE(normalClassTypeId)
{
	TestModule::BasePtr base = std::make_shared<TestModule::Base>(1);
	BOOST_REQUIRE(base);
	Slicer::ModelPart::CreateFor(&base, [](auto && a) {
		BOOST_REQUIRE(a);
		auto baseType = a->GetTypeId();
		BOOST_REQUIRE(!baseType);
	});
}

BOOST_AUTO_TEST_CASE(normalSubClassTypeId)
{
	TestModule::BasePtr base = std::make_shared<TestModule::D1>(1, 2);
	BOOST_REQUIRE(base);
	Slicer::ModelPart::CreateFor(&base, [](auto && a) {
		BOOST_REQUIRE(a);
		auto baseType = a->GetTypeId();
		BOOST_REQUIRE(baseType);
		BOOST_REQUIRE_EQUAL(*baseType, "::TestModule::D1");
	});
}

BOOST_AUTO_TEST_CASE(normalSubSubClassTypeId)
{
	TestModule::BasePtr base = std::make_shared<TestModule::D3>(1, 2, 3);
	BOOST_REQUIRE(base);
	Slicer::ModelPart::CreateFor(&base, [](auto && a) {
		BOOST_REQUIRE(a);
		auto baseType = a->GetTypeId();
		BOOST_REQUIRE(baseType);
		BOOST_REQUIRE_EQUAL(*baseType, "::TestModule::D3");
	});
}

BOOST_AUTO_TEST_CASE(localClassTypeId)
{
	Locals::LocalClassPtr base = std::make_shared<Locals::LocalClass>(1, "One");
	BOOST_REQUIRE(base);
	Slicer::ModelPart::CreateFor(&base, [](auto && a) {
		BOOST_REQUIRE(a);
		auto baseType = a->GetTypeId();
		BOOST_REQUIRE(!baseType);
	});
}

BOOST_AUTO_TEST_CASE(localSubClassTypeId)
{
	Locals::LocalClassPtr base = std::make_shared<Locals::LocalSubClass>(1, "One", 3.1);
	BOOST_REQUIRE(base);
	Slicer::ModelPart::CreateFor(&base, [](auto && a) {
		BOOST_REQUIRE(a);
		auto baseType = a->GetTypeId();
		BOOST_REQUIRE(baseType);
		BOOST_REQUIRE_EQUAL(*baseType, "::Locals::LocalSubClass");
	});
}

BOOST_AUTO_TEST_CASE(localSubSubClassTypeId)
{
	Locals::LocalClassPtr base = std::make_shared<Locals::LocalSub2Class>(1, "One", 3.1, 1);
	BOOST_REQUIRE(base);
	Slicer::ModelPart::CreateFor(&base, [](auto && a) {
		BOOST_REQUIRE(a);
		auto baseType = a->GetTypeId();
		BOOST_REQUIRE(baseType);
		BOOST_REQUIRE_EQUAL(*baseType, "::Locals::LocalSub2Class");
	});
}
