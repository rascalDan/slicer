#ifndef SLICER_ICE_H
#define SLICER_ICE_H

#include <Ice/BuiltinSequences.h>
#include <Ice/CommunicatorF.h>
#include <iosfwd>
#include <slicer/modelParts.h>
#include <slicer/serializer.h>
#include <visibility.h>

namespace Slicer {
	class IceBase {
	public:
		virtual ~IceBase();

	protected:
		IceBase();

		Ice::CommunicatorPtr ic;
	};

	class DLL_PUBLIC IceBlobSerializer : public Serializer, protected IceBase {
	public:
		void Serialize(ModelPartForRootPtr) override;

	protected:
		Ice::ByteSeq blob;
	};

	class DLL_PUBLIC IceStreamSerializer : public IceBlobSerializer {
	public:
		explicit IceStreamSerializer(std::ostream &);

		void Serialize(ModelPartForRootPtr) override;

	protected:
		std::ostream & strm;
	};

	class DLL_PUBLIC IceBlobDeserializer : public Deserializer, protected IceBase {
	public:
		explicit IceBlobDeserializer(const Ice::ByteSeq &);

		void Deserialize(ModelPartForRootPtr) override;

	protected:
		const Ice::ByteSeq & refblob;
	};

	class DLL_PUBLIC IceStreamDeserializer : public IceBlobDeserializer {
	public:
		explicit IceStreamDeserializer(std::istream &);

		void Deserialize(ModelPartForRootPtr) override;

	protected:
		std::istream & strm;
		Ice::ByteSeq blob;
	};
}

#endif
