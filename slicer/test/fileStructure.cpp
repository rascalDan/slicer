#include "fileStructure.h"
#include <boost/test/unit_test.hpp>
#include <boost/filesystem/convenience.hpp>

#define XSTR(s) STR(s)
#define STR(s) #s
const boost::filesystem::path root(XSTR(ROOT));

FileStructure::FileStructure() :
	me(boost::filesystem::canonical("/proc/self/exe")),
	base("types"),
	bjamout("bin" / me.parent_path().parent_path().leaf() / me.parent_path().leaf()),
	root(::root),
	included(root / "included"),
	slice(fs::change_extension(root / base, ".ice")),
	tmp(root / "bin" / "slicer")
{
	fs::create_directory(tmp);
}

FileStructure::~FileStructure()
{
}

