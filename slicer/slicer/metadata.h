#ifndef SLICER_METADATA_H
#define SLICER_METADATA_H

#include <string>
#include <list>
#include <vector>
#include <boost/optional.hpp>

#pragma GCC visibility push(default)
namespace Slicer {
	// Flags
	bool metaDataFlagSet(const std::list<std::string> &, const std::string & flag);
	bool metaDataFlagNotSet(const std::list<std::string> &, const std::string & flag);
	// Values
	boost::optional<std::string> metaDataValue(const std::string & prefix, const std::list<std::string> & metadata);
	std::list<std::string> metaDataValues(const std::string & prefix, const std::list<std::string> & metadata);
	std::vector<std::string> metaDataSplit(const std::string & metadata);
}
#pragma GCC visibility pop

#endif

