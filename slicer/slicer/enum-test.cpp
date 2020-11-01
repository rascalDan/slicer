#include "enumMap.h"

namespace test {
	enum class Es { one, two, three };
	const std::string one {"one"}, two {"two"}, three {"three"};

	constexpr Slicer::EnumMapImpl<Es, 3> em {{{
			{Es::one, "one", &one},
			{Es::two, "two", &two},
			{Es::three, "three", &three},
	}}};

	static_assert(em.arr.size() == 3);
	static_assert(em.arr[0].value == Es::one);
	static_assert(em.arr[1].name == "two");
	static_assert(em.arr[1].nameStr == &two);
	static_assert(em.arr[2].value == Es::three);

	static_assert(em.find<Slicer::EnumMapKey::Name>("one")->value == Es::one);
	static_assert(em.find<Slicer::EnumMapKey::Name>("three")->value == Es::three);
	static_assert(em.find<Slicer::EnumMapKey::Value>(Es::one)->name == "one");
	static_assert(em.find<Slicer::EnumMapKey::Value>(Es::three)->name == "three");
	static_assert(!em.find<Slicer::EnumMapKey::Name>("four"));
}
