#ifndef SLICER_DB_SQLCOMMON_H
#define SLICER_DB_SQLCOMMON_H

#include <string>
#include <modelParts.h>

namespace Slicer {
	bool isPKey(HookCommonPtr);
	bool isAuto(HookCommonPtr);
	bool isNotAuto(HookCommonPtr);
	bool isBind(HookCommonPtr);
	bool isValue(HookCommonPtr);
}

#endif

