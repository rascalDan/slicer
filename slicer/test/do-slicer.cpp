#include <slicer/parser.h>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include "../../libmisc/misc.h"
#include <types.h>
#include "helpers.h"

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

	// Slicer
	const fs::path cpp = fs::change_extension(tmp / base, ".cpp");
	fprintf(stderr, "cpp --- %s\n", cpp.string().c_str());
	fs::remove(cpp);
	Slicer::Slicer::Apply(slice, cpp);

	// Compile
	const fs::path obj = fs::change_extension(tmp / base, ".o");
	fprintf(stderr, "obj --- %s\n", obj.string().c_str());
	system(stringbf(
					"g++ -Os -fPIC -c -std=c++0x -I tmp -I /usr/include/Ice -I /usr/include/IceUtil -I %s -I %s %s -o %s",
					bjamout,
					root / "..",
					cpp, obj));

	// Link
	const fs::path so = fs::change_extension(tmp / ("libslicer" + slice.filename().string()), ".so");
	fprintf(stderr, "so ---- %s\n", so.string().c_str());
	system(stringbf(
					"g++ -shared -lIce -lIceUtil %s/lib%s.so %s -o %s",
					bjamout, base,
					obj, so));

	// Load
	fprintf(stderr, "load -- %s\n", so.string().c_str());
	auto handle = loadlib(so);

	// Unload
	fprintf(stderr, "unload  %p\n", handle);
	closelib(handle);

	return 0;
}

