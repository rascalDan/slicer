#include <slicer/parser.h>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int
main(int argc, char ** argv)
{
	boost::filesystem::path slice;
	boost::filesystem::path cpp;

	po::options_description opts("Slicer options");
	opts.add_options()
		("help,h", "Show this help message")
		("slice,i", po::value(&slice), "Input ICE Slice file")
		("cpp,o", po::value(&cpp), "Output C++ file");

	po::positional_options_description p;
	p.add("slice", 1);
	p.add("cpp", 1);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(opts).positional(p).run(), vm);
	po::notify(vm);

	if (vm.count("help") || slice.empty() || cpp.empty()) {
		std::cout << opts << std::endl;
		return 1;
	}

	Slicer::Slicer::Apply(slice, cpp);

	return 0;
}



