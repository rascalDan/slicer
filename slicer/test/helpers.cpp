#include "helpers.h"
#include <boost/test/test_tools.hpp>
#include <fstream>

void
diff(const std::filesystem::path & left, const std::filesystem::path & right)
{
	std::ifstream fl(left.string());
	std::ifstream fr(right.string());

	std::string l, r;
	std::copy_if(std::istreambuf_iterator<char>(fl), std::istreambuf_iterator<char>(), back_inserter(l), [](char x) {
		return !isspace(x);
	});
	std::copy_if(std::istreambuf_iterator<char>(fr), std::istreambuf_iterator<char>(), back_inserter(r), [](char x) {
		return !isspace(x);
	});

	BOOST_REQUIRE_EQUAL(l, r);
}
