#include <tool/parser.h>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int
main(int argc, char ** argv)
{
	Slicer::Slicer slicer;

	po::options_description opts("Slicer options");
	opts.add_options()
		("help,h", "Show this help message")
		("include,I", po::value(&slicer.includes), "Add include directory to search path")
		("ice", po::value(&slicer.allowIcePrefix)->default_value(slicer.allowIcePrefix)->zero_tokens(), "Allow reserved Ice prefix in Slice identifiers")
		("headerPrefix", po::value(&slicer.headerPrefix)->default_value(slicer.headerPrefix), "Prefix path for Slicer C++ #includes")
		("slice,i", po::value(&slicer.slicePath), "Input ICE Slice file")
		("cpp,o", po::value(&slicer.cppPath), "Output C++ file");

	po::positional_options_description p;
	p.add("slice", 1);
	p.add("cpp", 1);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(opts).positional(p).run(), vm);
	po::notify(vm);

	if (vm.count("help") || slicer.slicePath.empty() || slicer.cppPath.empty()) {
		// LCOV_EXCL_START
		std::cout << opts << std::endl;
		return 1;
		// LCOV_EXCL_STOP
	}
	slicer.Execute();

	return 0;
}

