#include "serializer.h"
#include "Ice/Communicator.h"
#include "Ice/Initialize.h"

NAMEDFACTORY("application/ice", Slicer::IceStreamSerializer, Slicer::StreamSerializerFactory);
NAMEDFACTORY("application/ice", Slicer::IceStreamDeserializer, Slicer::StreamDeserializerFactory);

namespace Slicer {
	Ice::StringSeq empty;

	IceBase::IceBase() : ic(Ice::initialize(empty)) { }

	IceBase::~IceBase()
	{
		ic->destroy();
	}
	IceBlobSerializer::IceBlobSerializer(Ice::ByteSeq & b) : refblob(b) { }

	void
	IceBlobSerializer::Serialize(ModelPartForRootPtr mp)
	{
		Ice::OutputStream s(ic);
		mp->Write(s);
		s.finished(refblob);
	}

	IceStreamSerializer::IceStreamSerializer(std::ostream & os) : IceBlobSerializer(blob), strm(os) { }

	void
	IceStreamSerializer::Serialize(ModelPartForRootPtr mp)
	{
		IceBlobSerializer::Serialize(mp);
		strm.write(reinterpret_cast<const char *>(blob.data()), blob.size());
	}

	IceBlobDeserializer::IceBlobDeserializer(const Ice::ByteSeq & b) : refblob(b) { }

	void
	IceBlobDeserializer::Deserialize(ModelPartForRootPtr mp)
	{
		Ice::InputStream s(ic, refblob);
		mp->Read(s);
	}

	IceStreamDeserializer::IceStreamDeserializer(std::istream & is) : IceBlobDeserializer(blob), strm(is) { }

	void
	IceStreamDeserializer::Deserialize(ModelPartForRootPtr mp)
	{
		blob.assign(std::istreambuf_iterator<char>(strm), std::istreambuf_iterator<char>());
		IceBlobDeserializer::Deserialize(mp);
	}
}
