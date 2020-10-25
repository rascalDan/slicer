#include "hookMap.h"
#include "modelPartsTypes.impl.h"

#include <string>

struct S {
	int aa;
	int aA;
	int Aa;
	int AA;
	std::string b;
};
const std::string aa {"aa"}, aA {"aA"}, Aa {"Aa"}, AA {"AA"}, b {"b"};

using C = Slicer::ModelPartForComplex<S>;
constexpr C::Hook<int, Slicer::ModelPartForSimple<int>, 0> haa {&S::aa, "aa", "aa", &aa};
constexpr C::Hook<int, Slicer::ModelPartForSimple<int>, 1> haA {&S::aA, "aA", "aa", &aA, "md1"};
constexpr C::Hook<int, Slicer::ModelPartForSimple<int>, 3> hAa {&S::Aa, "Aa", "aa", &Aa, "md2", "md3", "md4"};
constexpr C::Hook<int, Slicer::ModelPartForSimple<int>> hAA {&S::AA, "AA", "aa", &AA};
constexpr C::Hook<std::string, Slicer::ModelPartForSimple<std::string>, 0> hb {&S::b, "b", "b", &b};
constexpr Slicer::HooksImpl<S, 5> h {{{&haa, &haA, &hAa, &hAA, &hb}}};

static_assert(h.arr.size() == 5);
static_assert(h.arr[0]->name == "aa");
static_assert(h.arr[0]->nameLower == "aa");
static_assert(h.arr[0]->nameStr == &aa);
static_assert(h.arr[1]->name == "aA");
static_assert(h.arr[1]->nameLower == "aa");
static_assert(h.arr[1]->nameStr == &aA);
static_assert(h.arr[4]->name == "b");
static_assert(h.arr[4]->nameLower == "b");
static_assert(h.arr[4]->nameStr == &b);

constexpr auto aas = h.equal_range("aa");
constexpr auto bbs = h.equal_range("bb");
static_assert(aas.begin() != aas.end());
static_assert(bbs.begin() == bbs.end());
static_assert(aas.begin()->name == "aa");

constexpr auto aa0 = aas.begin();
static_assert(aa0->name == "aa");
constexpr auto aa1 = aa0 + 1;
static_assert(aa1 == aas.end());
static_assert(std::distance(aas.begin(), aas.end()) == 1);
static_assert(std::distance(bbs.begin(), bbs.end()) == 0);

constexpr auto laas = h.equal_range_lower("aa");
constexpr auto lAAs = h.equal_range_lower("AA");
static_assert(std::distance(laas.begin(), laas.end()) == 4);
static_assert(std::distance(lAAs.begin(), lAAs.end()) == 0);
static_assert(laas.begin()->name == "aa");
static_assert((laas.begin() + 1)->name == "aA");
static_assert((laas.begin() + 2)->name == "Aa");
static_assert((laas.begin() + 3)->name == "AA");
static_assert((laas.begin() + 4) == laas.end());
