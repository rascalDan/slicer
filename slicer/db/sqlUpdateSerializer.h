#ifndef SLICER_DB_SQLUPDATESERIALIZER_H
#define SLICER_DB_SQLUPDATESERIALIZER_H

#include <slicer/serializer.h>
#include <connection.h>
#include <visibility.h>

namespace Slicer {
	class NoRowsFound : public std::runtime_error {
		public:
			NoRowsFound();
	};

	class DLL_PUBLIC SqlUpdateSerializer : public Slicer::Serializer {
		public:
			typedef boost::shared_ptr<DB::ModifyCommand> ModifyPtr;

			SqlUpdateSerializer(DB::Connection *, const std::string & tableName);

			virtual void Serialize(Slicer::ModelPartPtr) override;

		protected:
			void SerializeObject(Slicer::ModelPartPtr) const;
			void SerializeSequence(Slicer::ModelPartPtr) const;
			ModifyPtr createUpdate(Slicer::ModelPartPtr) const;
			static void bindObjectAndExecute(Slicer::ModelPartPtr, DB::ModifyCommand *);

			DB::Connection * connection;
			const std::string tableName;
	};
}

#endif

