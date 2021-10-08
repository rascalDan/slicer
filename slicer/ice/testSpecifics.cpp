#define BOOST_TEST_MODULE ice_specifics
#include <boost/test/unit_test.hpp>

#include "classes.h"
#include "serializer.h"
#include "structs.h"
#include <Ice/Comparable.h>
#include <Ice/Config.h>
#include <Ice/Optional.h>
#include <functional>
#include <iosfwd>
#include <memory>
#include <slicer/slicer.h>
#include <string>
#include <typeinfo>
// IWYU pragma: no_forward_declare Slicer::IceStreamDeserializer

// LCOV_EXCL_START
// cppcheck-suppress unknownMacro
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestModule::IsoDate)
// LCOV_EXCL_STOP

template<typename X>
void
testCompare(const X & x)
{
	BOOST_TEST_CONTEXT(typeid(X).name()) {
		std::stringstream strm;
		Slicer::SerializeAny<Slicer::IceStreamSerializer>(x, strm);
		auto x2 = Slicer::DeserializeAny<Slicer::IceStreamDeserializer, X>(strm);
		BOOST_REQUIRE_EQUAL(x, x2);
	}
}

template<typename X>
void
testCompare(const X & x, const std::function<bool(const X &, const X &)> & cmp)
{
	BOOST_TEST_CONTEXT(typeid(X).name()) {
		std::stringstream strm;
		Slicer::SerializeAny<Slicer::IceStreamSerializer>(x, strm);
		auto x2 = Slicer::DeserializeAny<Slicer::IceStreamDeserializer, X>(strm);
		BOOST_REQUIRE(cmp(x, x2));
	}
}

BOOST_AUTO_TEST_CASE(builtins)
{
	testCompare<std::string>("some string value");
	testCompare<bool>(true);
	testCompare<bool>(false);
	testCompare<Ice::Byte>(10);
	testCompare<Ice::Byte>(0);
	testCompare<Ice::Short>(-5);
	testCompare<Ice::Short>(534);
	testCompare<Ice::Int>(-433);
	testCompare<Ice::Int>(3434090);
	testCompare<Ice::Long>(-4000033);
	testCompare<Ice::Long>(343409898900);
	testCompare<Ice::Float>(-3.14F);
	testCompare<Ice::Float>(3.14F);
	testCompare<Ice::Double>(-3.14159);
	testCompare<Ice::Double>(3.14159);
}

template<typename X>
void
testCompareOptional(const X & d)
{
	BOOST_TEST_CONTEXT(typeid(X).name()) {
		std::stringstream strm;
		Ice::optional<X> x;
		Slicer::SerializeAny<Slicer::IceStreamSerializer>(x, strm);
		auto x2 = Slicer::DeserializeAny<Slicer::IceStreamDeserializer, Ice::optional<X>>(strm);
		BOOST_REQUIRE(!x2);
		x = d;
		Slicer::SerializeAny<Slicer::IceStreamSerializer>(x, strm);
		auto x3 = Slicer::DeserializeAny<Slicer::IceStreamDeserializer, Ice::optional<X>>(strm);
		BOOST_REQUIRE_EQUAL(d, *x3);
	}
}

BOOST_AUTO_TEST_CASE(optionalBuiltins)
{
	testCompareOptional<std::string>("some string value");
	testCompareOptional<bool>(true);
	testCompareOptional<bool>(false);
	testCompareOptional<Ice::Byte>(10);
	testCompareOptional<Ice::Byte>(0);
	testCompareOptional<Ice::Short>(-5);
	testCompareOptional<Ice::Short>(534);
	testCompareOptional<Ice::Int>(-433);
	testCompareOptional<Ice::Int>(3434090);
	testCompareOptional<Ice::Long>(-4000033);
	testCompareOptional<Ice::Long>(343409898900);
	testCompareOptional<Ice::Float>(-3.14F);
	testCompareOptional<Ice::Float>(3.14F);
	testCompareOptional<Ice::Double>(-3.14159);
	testCompareOptional<Ice::Double>(3.14159);
}

BOOST_AUTO_TEST_CASE(classes)
{
	TestModule::BuiltInsPtr x = std::make_shared<TestModule::BuiltIns>();
	x->mbool = true;
	x->mbyte = 14;
	x->mshort = 31434;
	x->mint = 324324234;
	x->mlong = 343242342343243;
	x->mfloat = 3434.32432F;
	x->mdouble = 3423423423.42342342343;
	x->mstring = "sdfsf432423";
	testCompare<TestModule::BuiltInsPtr>(x, [](const auto & a, const auto & b) {
		return a->mbool == b->mbool && a->mbyte == b->mbyte && a->mshort == b->mshort && a->mint == b->mint
				&& a->mlong == b->mlong && a->mfloat == b->mfloat && a->mdouble == b->mdouble
				&& a->mstring == b->mstring;
	});
}

BOOST_AUTO_TEST_CASE(structes)
{
	TestModule::IsoDate date({2016, 10, 3});
	testCompare(date);
	testCompareOptional(date);
}
