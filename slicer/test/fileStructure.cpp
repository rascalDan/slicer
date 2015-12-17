#include "fileStructure.h"
#include <boost/test/unit_test.hpp>
#include <boost/filesystem/convenience.hpp>
#include <definedDirs.h>

FileStructure::FileStructure() :
	me(selfExe),
	base("types"),
	bjamout("bin" / buildVariant),
	root(rootDir),
	included(root / "included"),
	slice(fs::change_extension(root / base, ".ice")),
	tmp(root / "bin" / "slicer")
{
	fs::create_directory(tmp);
}

FileStructure::~FileStructure()
{
}

