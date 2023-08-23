#include "serializer.h"
#include <Ice/Communicator.h>
#include <Ice/Initialize.h>
#include <Ice/InputStream.h>
#include <Ice/OutputStream.h>
#include <factory.h>
#include <istream>
#include <iterator>
#include <memory>
#include <slicer/modelParts.h>
#include <slicer/serializer.h>

NAMEDFACTORY("application/ice", Slicer::IceStreamSerializer, Slicer::StreamSerializerFactory)
NAMEDFACTORY("application/ice", Slicer::IceStreamDeserializer, Slicer::StreamDeserializerFactory)

namespace Slicer {
	IceBase::IceBase() : ic(Ice::initialize()) { }

	IceBase::~IceBase()
	{
		ic->destroy();
	}

	void
	IceBlobSerializer::Serialize(ModelPartForRootParam mp)
	{
		Ice::OutputStream s(ic);
		mp->Write(s);
		s.finished(blob);
	}

	IceStreamSerializer::IceStreamSerializer(std::ostream & os) : strm(os) { }

	void
	IceStreamSerializer::Serialize(ModelPartForRootParam mp)
	{
		IceBlobSerializer::Serialize(mp);
		strm.write(reinterpret_cast<const char *>(blob.data()), static_cast<std::streamsize>(blob.size()));
	}

	IceBlobDeserializer::IceBlobDeserializer(const Ice::ByteSeq & b) : refblob(b) { }

	void
	IceBlobDeserializer::Deserialize(ModelPartForRootParam mp)
	{
		Ice::InputStream s(ic, refblob);
		mp->Read(s);
	}

	IceStreamDeserializer::IceStreamDeserializer(std::istream & is) : IceBlobDeserializer(blob), strm(is) { }

	void
	IceStreamDeserializer::Deserialize(ModelPartForRootParam mp)
	{
		blob.assign(std::istreambuf_iterator<char>(strm), std::istreambuf_iterator<char>());
		IceBlobDeserializer::Deserialize(mp);
	}
}
