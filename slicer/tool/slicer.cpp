#include <tool/parser.h>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int
main(int argc, char ** argv)
{
	boost::filesystem::path slice;
	boost::filesystem::path cpp;
	std::vector<boost::filesystem::path> includes;
	bool allowIcePrefix;

	po::options_description opts("Slicer options");
	opts.add_options()
		("help,h", "Show this help message")
		("include,I", po::value(&includes), "Add include directory to search path")
		("ice", po::value(&allowIcePrefix)->default_value(false)->zero_tokens(), "Allow reserved Ice prefix in Slice identifiers")
		("slice,i", po::value(&slice), "Input ICE Slice file")
		("cpp,o", po::value(&cpp), "Output C++ file");

	po::positional_options_description p;
	p.add("slice", 1);
	p.add("cpp", 1);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(opts).positional(p).run(), vm);
	po::notify(vm);

	if (vm.count("help") || slice.empty() || cpp.empty()) {
		// LCOV_EXCL_START
		std::cout << opts << std::endl;
		return 1;
		// LCOV_EXCL_STOP
	}
	Slicer::Slicer::Args args;
	for(const auto & include : includes) {
		args.push_back("-I" + include.string());
	}
	Slicer::Slicer::Apply(slice, cpp, args, allowIcePrefix);

	return 0;
}



