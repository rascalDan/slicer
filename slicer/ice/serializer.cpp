#include "serializer.h"
#include "Ice/Initialize.h"
#include "Ice/Communicator.h"

NAMEDFACTORY("application/ice", Slicer::IceStreamSerializer, Slicer::StreamSerializerFactory);
NAMEDFACTORY("application/ice", Slicer::IceStreamDeserializer, Slicer::StreamDeserializerFactory);

namespace Slicer {
	Ice::StringSeq empty;

	IceBase::IceBase() :
		ic(Ice::initialize(empty))
	{
	}

	IceBase::~IceBase()
	{
		ic->destroy();
	}
	IceBlobSerializer::IceBlobSerializer(Ice::ByteSeq & b) :
		blob(b)
	{
	}

	void
	IceBlobSerializer::Serialize(ModelPartForRootPtr mp)
	{
		Ice::OutputStream s(ic);
		mp->Write(s);
		s.finished(blob);
	}

	IceStreamSerializer::IceStreamSerializer(std::ostream & os) :
		IceBlobSerializer(blob),
		strm(os)
	{
	}

	void
	IceStreamSerializer::Serialize(ModelPartForRootPtr mp)
	{
		IceBlobSerializer::Serialize(mp);
		strm.write((const char *)&blob.front(), blob.size());
	}

	IceBlobDeserializer::IceBlobDeserializer(const Ice::ByteSeq & b) :
		blob(b)
	{
	}

	void
	IceBlobDeserializer::Deserialize(ModelPartForRootPtr mp)
	{
		Ice::InputStream s(ic, blob);
		mp->Read(s);
	}

	IceStreamDeserializer::IceStreamDeserializer(std::istream & is) :
		IceBlobDeserializer(blob),
		strm(is)
	{
	}

	void
	IceStreamDeserializer::Deserialize(ModelPartForRootPtr mp)
	{
		blob.assign(std::istreambuf_iterator<char>(strm), std::istreambuf_iterator<char>());
		IceBlobDeserializer::Deserialize(mp);
	}
}

