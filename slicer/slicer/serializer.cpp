#define BOOST_FILESYSTEM_DYN_LINK
#define BOOST_FILESYSTEM_SOURCE
#include "serializer.h"
#include <factory.impl.h>

INSTANTIATEFACTORY(Slicer::Serializer, std::ostream &);
INSTANTIATEFACTORY(Slicer::Deserializer, std::istream &);
INSTANTIATEFACTORY(Slicer::Serializer, const boost::filesystem::path &);
INSTANTIATEFACTORY(Slicer::Deserializer, const boost::filesystem::path &);

