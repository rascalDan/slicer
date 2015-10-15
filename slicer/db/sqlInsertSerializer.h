#ifndef SLICER_DB_SQLINSERTSERIALIZER_H
#define SLICER_DB_SQLINSERTSERIALIZER_H

#include <slicer/serializer.h>
#include <connection.h>
#include <visibility.h>

namespace Slicer {
	class DLL_PUBLIC SqlInsertSerializer : public Slicer::Serializer {
		public:
			typedef boost::shared_ptr<DB::ModifyCommand> ModifyPtr;

			SqlInsertSerializer(DB::Connection *, const std::string & tableName);

			virtual void Serialize(Slicer::ModelPartPtr) override;

		protected:
			void SerializeObject(Slicer::ModelPartPtr) const;
			void SerializeSequence(Slicer::ModelPartPtr) const;
			ModifyPtr createInsert(Slicer::ModelPartPtr) const;

			DB::Connection * connection;
			const std::string tableName;
	};
}

#endif

