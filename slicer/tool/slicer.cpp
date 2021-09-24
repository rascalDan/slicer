#include "parser.h"
#include <array>
#include <boost/algorithm/string/find_iterator.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/program_options.hpp>
#include <compileTimeFormatter.h>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
// IWYU pragma: no_include <utility>
// IWYU pragma: no_include <boost/algorithm/string/compare.hpp>
// IWYU pragma: no_include <boost/core/addressof.hpp>
// IWYU pragma: no_include <boost/detail/basic_pointerbuf.hpp>
// IWYU pragma: no_include <boost/function/function_base.hpp>
// IWYU pragma: no_include <boost/iterator/iterator_facade.hpp>
// IWYU pragma: no_include <boost/lexical_cast.hpp>
// IWYU pragma: no_include <boost/range/const_iterator.hpp>
// IWYU pragma: no_include <boost/range/iterator_range_core.hpp>
// IWYU pragma: no_include <boost/type_index/type_index_facade.hpp>

namespace po = boost::program_options;
using namespace AdHoc::literals;

static std::string
defaultPostProcessor()
{
	constexpr std::array<const std::pair<std::string_view, std::string_view>, 1> pps {{
			{"clang-format", "-i"},
	}};
	const std::string_view path {getenv("PATH")};
	const auto pathBegin = make_split_iterator(path, first_finder(":", boost::is_equal()));
	for (const auto & [cmd, opts] : pps) {
		for (auto p = pathBegin; p != decltype(pathBegin) {}; ++p) {
			if (std::filesystem::exists(std::filesystem::path(p->begin(), p->end()) / cmd)) {
				return "%? %?"_fmt(cmd, opts);
			}
		}
	}
	return "";
}

int
main(int argc, char ** argv)
{
	Slicer::Slicer slicer;
	std::string post;

	po::options_description opts("Slicer options");
	// clang-format off
	opts.add_options()
		("help,h", "Show this help message")
		("include,I", po::value(&slicer.includes), "Add include directory to search path")
		// NOLINTNEXTLINE(clang-analyzer-optin.cplusplus.VirtualCall)
		("headerPrefix", po::value(&slicer.headerPrefix)->default_value(slicer.headerPrefix), "Prefix path for Slicer C++ #includes")
		("post,p", po::value(&post)->default_value(defaultPostProcessor()), "Post-process command")
		("slice,i", po::value(&slicer.slicePath), "Input ICE Slice file")
		("cpp,o", po::value(&slicer.cppPath), "Output C++ file");
	// clang-format on

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

	if (!post.empty()) {
		return system("%? %?"_fmt(post, slicer.cppPath).c_str());
	}

	return 0;
}
