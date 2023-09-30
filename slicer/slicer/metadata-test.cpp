#include "metadata.h"
#include <array>
#include <optional>
#include <string_view>

namespace test {
	constexpr auto rc = [](std::string_view sv) {
		return Slicer::MetaData<>::remove_colons(sv);
	};
	static_assert(rc("scope") == "scope");
	static_assert(rc("scope:") == "scope");
	static_assert(rc("scope::") == "scope");
	static_assert(rc("scope:sub:") == "scope:sub");
	static_assert(rc("scope:sub::") == "scope:sub");
	static_assert(Slicer::MetaData<>::in_scope("scope", "scope"));
	static_assert(!Slicer::MetaData<>::in_scope("scope2", "scope"));
	static_assert(!Slicer::MetaData<>::in_scope("scope", "scope2"));
	static_assert(Slicer::MetaData<>::in_scope("scope:sub", "scope"));
	static_assert(Slicer::MetaData<>::in_scope("scope:sub:subsub", "scope"));
	static_assert(Slicer::MetaData<>::in_scope("scope:sub:subsub", "scope:sub"));
	static_assert(!Slicer::MetaData<>::in_scope("scope:sub2:subsub", "scope:sub"));
	static_assert(!Slicer::MetaData<>::in_scope("scope:sub:subsub", "scope:sub2"));

	constexpr Slicer::MetaDataImpl<4> md {{{
			"slicer:yes",
			"slicer:no",
			"slicer:sub:scope",
			"notslicer:dontcare",
	}}};

	static_assert(md.arr[0].first == "slicer:yes");
	static_assert(md.arr[0].second == "slicer");
	static_assert(md.arr[2].first == "slicer:sub:scope");
	static_assert(md.arr[2].second == "slicer:sub");

	static_assert(md.flagSet("slicer:yes"));
	static_assert(md.flagSet("slicer:no"));
	static_assert(md.flagNotSet("slicer:chickens"));
	static_assert(md.value("slicer:").has_value());
	static_assert(md.value("slicer:").value() == "yes");
	static_assert(md.value("slicer").has_value());
	static_assert(md.value("slicer").value() == "yes");
	static_assert(md.value("slicer:sub:").has_value());
	static_assert(md.value("slicer:sub:").value() == "scope");
	static_assert(md.value("slicer:sub").has_value());
	static_assert(md.value("slicer:sub").value() == "scope");
	static_assert(!md.value("nope:").has_value());
}
