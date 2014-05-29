#include <slicer/parser.h>

int
main(int argc, char ** argv)
{
	if (argc < 3) {
		fprintf(stderr, "slicer <input.ice> <output.cpp>\n");
		return 1;
	}

	const boost::filesystem::path slice = argv[1];
	const boost::filesystem::path cpp = argv[2];

	Slicer::Slicer::Apply(slice, cpp);

	return 0;
}
