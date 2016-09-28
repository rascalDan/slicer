#ifndef SLICER_DB_SQLUPDATESERIALIZER_H
#define SLICER_DB_SQLUPDATESERIALIZER_H

#include <slicer/serializer.h>
#include <connection.h>
#include <visibility.h>

namespace Slicer {
	class DLL_PUBLIC SqlUpdateSerializer : public Slicer::Serializer {
		public:
			typedef boost::shared_ptr<DB::ModifyCommand> ModifyPtr;

			SqlUpdateSerializer(DB::Connection * const, const std::string & tableName);

			virtual void Serialize(Slicer::ModelPartForRootPtr) override;

		protected:
			void SerializeObject(Slicer::ModelPartPtr) const;
			void SerializeSequence(Slicer::ModelPartPtr) const;
			ModifyPtr createUpdate(Slicer::ModelPartPtr) const;
			static void bindObjectAndExecute(Slicer::ModelPartPtr, DB::ModifyCommand *);

			DB::Connection * const connection;
			const std::string tableName;
	};
}

#endif

