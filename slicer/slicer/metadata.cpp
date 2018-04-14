#include "metadata.h"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

bool
Slicer::metaDataFlagSet(const std::list<std::string> & md, const std::string & flag)
{
	return std::find(md.cbegin(), md.cend(), flag) != md.cend();
}

bool
Slicer::metaDataFlagNotSet(const std::list<std::string> & md, const std::string & flag)
{
	return std::find(md.cbegin(), md.cend(), flag) == md.cend();
}

std::list<std::string>
Slicer::metaDataValues(const std::string & prefix, const std::list<std::string> & metadata)
{
	std::list<std::string> mds;
	for (const auto & md : metadata) {
		if (boost::algorithm::starts_with(md, prefix)) {
			mds.push_back(md.substr(prefix.length()));
		}
	}
	return mds;
}

std::vector<std::string>
Slicer::metaDataSplit(const std::string & metadata)
{
	std::vector<std::string> parts;
	boost::algorithm::split(parts, metadata, boost::algorithm::is_any_of(":"), boost::algorithm::token_compress_off);
	return parts;
}

std::optional<std::string>
Slicer::metaDataValue(const std::string & prefix, const std::list<std::string> & metadata)
{
	for (const auto & md : metadata) {
		if (boost::algorithm::starts_with(md, prefix)) {
			return md.substr(prefix.length());
		}
	}
	return std::optional<std::string>();
}

