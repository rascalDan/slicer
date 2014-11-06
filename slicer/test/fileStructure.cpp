#include "fileStructure.h"
#include <boost/test/unit_test.hpp>
#include <boost/filesystem/convenience.hpp>

FileStructure::FileStructure() :
	me(boost::filesystem::canonical("/proc/self/exe")),
	base("types"),
	bjamout(me.parent_path()),
	root(me.parent_path().parent_path().parent_path().parent_path()),
	slice(fs::change_extension(root / base, ".ice")),
	tmp(root / "bin" / "slicer"),
	tmpf(tmp / "byFile"),
	tmph(tmp / "byHelper")
{
	fs::create_directory(tmp);
	fs::create_directory(tmpf);
	fs::create_directory(tmph);
}

FileStructure::~FileStructure()
{
}

