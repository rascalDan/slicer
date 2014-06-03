#include <slicer/parser.h>
#include <slicer/slicer.h>
#include <xml/serializer.h>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <types.h>

namespace fs = boost::filesystem;

int
main(int, char ** argv)
{
	const fs::path me = argv[0];
	const fs::path base = "types";
	const fs::path bjamout = me.parent_path();
	const fs::path root = me.parent_path().parent_path().parent_path().parent_path();
	fprintf(stderr, "root -- %s\n", root.string().c_str());
	const fs::path slice = fs::change_extension(root / base, ".ice");
	fprintf(stderr, "slice - %s\n", slice.string().c_str());

	// Tmp
	const fs::path tmp = root / "bin" / "slicer";
	fprintf(stderr, "tmp --- %s\n", tmp.string().c_str());
	fs::create_directory(tmp);

	// Execute
	TestModule::BuiltInsPtr p(new TestModule::BuiltIns());
	Slicer::Serialize<Slicer::Xml>(p, tmp / "out.xml");

	return 0;
}

