#include "serializer.h"
#include <factory.impl.h>

INSTANTIATEFACTORY(Slicer::Serializer, std::ostream &);
INSTANTIATEFACTORY(Slicer::Deserializer, std::istream &);
INSTANTIATEFACTORY(Slicer::Serializer, const std::filesystem::path &);
INSTANTIATEFACTORY(Slicer::Deserializer, const std::filesystem::path &);
