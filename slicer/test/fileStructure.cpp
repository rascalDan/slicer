#include "fileStructure.h"
#include <boost/test/unit_test.hpp>
#include <boost/filesystem/convenience.hpp>

FileStructure::FileStructure() :
	me(boost::filesystem::canonical("/proc/self/exe")),
	base("types"),
	bjamout(me.parent_path().parent_path().parent_path().leaf() / me.parent_path().parent_path().leaf() / me.parent_path().leaf()),
	root(me.parent_path().parent_path().parent_path().parent_path()),
	slice(fs::change_extension(root / base, ".ice")),
	tmp(root / "bin" / "slicer")
{
	fs::create_directory(tmp);
}

FileStructure::~FileStructure()
{
}

