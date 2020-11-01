#include "metadata.h"

namespace test {
	constexpr Slicer::MetaDataImpl<4> md {{{
			"slicer:yes",
			"slicer:no",
			"slicer:sub:scope",
			"notslicer:dontcare",
	}}};

	static_assert(md.flagSet("slicer:yes"));
	static_assert(md.flagSet("slicer:no"));
	static_assert(md.flagNotSet("slicer:chickens"));
	static_assert(md.value("slicer:").has_value());
	static_assert(md.value("slicer:").value() == "yes");
	static_assert(!md.value("nope:").has_value());
}
