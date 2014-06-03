#include "helpers.h"
#include <stdexcept>
#include <stdlib.h>
#include <dlfcn.h>

void
system(const std::string & cmd)
{
	if (system(cmd.c_str())) {
		fprintf(stderr, "Failed to execute:\n\t%s\n", cmd.c_str());
		throw std::runtime_error(cmd);
	}
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

