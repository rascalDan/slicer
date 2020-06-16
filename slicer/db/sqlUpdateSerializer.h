#ifndef SLICER_DB_SQLUPDATESERIALIZER_H
#define SLICER_DB_SQLUPDATESERIALIZER_H

#include <connection.h>
#include <slicer/serializer.h>
#include <visibility.h>

namespace Slicer {
	class DLL_PUBLIC SqlUpdateSerializer : public Slicer::Serializer {
	public:
		SqlUpdateSerializer(DB::Connection * const, std::string tableName);

		void Serialize(Slicer::ModelPartForRootPtr) override;

	protected:
		void SerializeObject(const Slicer::ModelPartPtr &) const;
		void SerializeSequence(const Slicer::ModelPartPtr &) const;
		DB::ModifyCommandPtr createUpdate(const Slicer::ModelPartPtr &) const;
		static void bindObjectAndExecute(const Slicer::ModelPartPtr &, DB::ModifyCommand *);

		DB::Connection * const connection;
		const std::string tableName;
	};
}

#endif
