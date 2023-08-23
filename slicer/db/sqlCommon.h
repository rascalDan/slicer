#ifndef SLICER_DB_SQLCOMMON_H
#define SLICER_DB_SQLCOMMON_H

namespace Slicer {
	class HookCommon;

	[[nodiscard]] bool isPKey(const HookCommon *) noexcept;
	[[nodiscard]] bool isAuto(const HookCommon *) noexcept;
	[[nodiscard]] bool isNotAuto(const HookCommon *) noexcept;
	[[nodiscard]] bool isBind(const HookCommon *) noexcept;
	[[nodiscard]] bool isValue(const HookCommon *) noexcept;
}

#endif
