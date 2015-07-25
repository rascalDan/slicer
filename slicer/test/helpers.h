#ifndef SLICER_TEST_HELPERS_H
#define SLICER_TEST_HELPERS_H

#include <string>
#include <boost/filesystem/path.hpp>

// These are just thin wrappers that throw exceptions
void
system(const std::string &);

void *
loadlib(const boost::filesystem::path &);

void
closelib(void *);

void
diff(const boost::filesystem::path & left, const boost::filesystem::path & right);

#endif

