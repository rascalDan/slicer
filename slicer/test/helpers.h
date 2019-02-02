#ifndef SLICER_TEST_HELPERS_H
#define SLICER_TEST_HELPERS_H

#include <string>
#include <visibility.h>
#include <filesystem>

// These are just thin wrappers that throw exceptions
DLL_PUBLIC
void
diff(const std::filesystem::path & left, const std::filesystem::path & right);

#endif

