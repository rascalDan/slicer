#include <slicer/parser.h>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>

int
main(int, char ** argv)
{
	const boost::filesystem::path slice = argv[1];
	Slicer::Slicer::Apply(slice, "/dev/null");

	return 0;
}

