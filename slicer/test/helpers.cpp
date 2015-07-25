#include "helpers.h"
#include <stdexcept>
#include <stdlib.h>
#include <dlfcn.h>
#include <fstream>
#include <boost/test/test_tools.hpp>

void
system(const std::string & cmd)
{
	if (system(cmd.c_str())) {
		fprintf(stderr, "Failed to execute:\n\t%s\n", cmd.c_str());
		throw std::runtime_error(cmd);
	}
}

void
diff(const boost::filesystem::path & left, const boost::filesystem::path & right)
{
	std::ifstream fl(left.string());
	std::ifstream fr(right.string());

	std::string l, r;
	std::copy_if(std::istreambuf_iterator<char>(fl), std::istreambuf_iterator<char>(), back_inserter(l), [](char x){ return !isspace(x); });
	std::copy_if(std::istreambuf_iterator<char>(fr), std::istreambuf_iterator<char>(), back_inserter(r), [](char x){ return !isspace(x); });

	BOOST_REQUIRE_EQUAL(l, r);
}

void *
loadlib(const boost::filesystem::path & so)
{
	auto handle = dlopen(so.string().c_str(), RTLD_NOW | RTLD_GLOBAL);
	if (!handle) {
		throw std::runtime_error(dlerror());
	}
	return handle;
}

void
closelib(void * handle)
{
	if (dlclose(handle)) {
		throw std::runtime_error(dlerror());
	}
}

