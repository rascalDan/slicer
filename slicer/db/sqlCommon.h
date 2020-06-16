#ifndef SLICER_DB_SQLCOMMON_H
#define SLICER_DB_SQLCOMMON_H

#include <modelParts.h>
#include <string>

namespace Slicer {
	bool isPKey(const HookCommon *);
	bool isAuto(const HookCommon *);
	bool isNotAuto(const HookCommon *);
	bool isBind(const HookCommon *);
	bool isValue(const HookCommon *);
}

#endif
