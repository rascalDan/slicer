#ifndef SLICER_DB_SQLUPDATESERIALIZER_H
#define SLICER_DB_SQLUPDATESERIALIZER_H

#include <command_fwd.h>
#include <slicer/modelParts.h>
#include <slicer/serializer.h>
#include <string>
#include <visibility.h>

namespace DB {
	class Connection;
	class ModifyCommand;
}

namespace Slicer {
	class DLL_PUBLIC SqlUpdateSerializer : public Slicer::Serializer {
	public:
		SqlUpdateSerializer(DB::Connection * const, std::string tableName);

		void Serialize(Slicer::ModelPartForRootPtr) override;

	protected:
		void SerializeObject(ModelPartParam) const;
		void SerializeSequence(ModelPartParam) const;
		[[nodiscard]] DB::ModifyCommandPtr createUpdate(ModelPartParam) const;
		static void bindObjectAndExecute(ModelPartParam, DB::ModifyCommand *);

		DB::Connection * const connection;
		const std::string tableName;
	};
}

#endif
