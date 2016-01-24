#ifndef SLICER_TEST_HELPERS_H
#define SLICER_TEST_HELPERS_H

#include <string>
#include <visibility.h>
#include <boost/filesystem/path.hpp>

// These are just thin wrappers that throw exceptions
DLL_PUBLIC
void
system(const std::string &);

DLL_PUBLIC
void *
loadlib(const boost::filesystem::path &);

DLL_PUBLIC
void
closelib(void *);

DLL_PUBLIC
void
diff(const boost::filesystem::path & left, const boost::filesystem::path & right);

#endif

