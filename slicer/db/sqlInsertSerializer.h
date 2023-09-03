#pragma once

#include <command_fwd.h>
#include <ostream>
#include <slicer/modelParts.h>
#include <slicer/serializer.h>
#include <string>
#include <visibility.h>

namespace DB {
	class Connection;
	class ModifyCommand;
}

namespace Slicer {
	class DLL_PUBLIC SqlInsertSerializer : public Slicer::Serializer {
	public:
		SqlInsertSerializer(DB::Connection * const, std::string tableName);

		void Serialize(ModelPartForRootParam) override;

	protected:
		void SerializeObject(ModelPartParam) const;
		void SerializeSequence(ModelPartParam) const;
		[[nodiscard]] DB::ModifyCommandPtr createInsert(ModelPartParam) const;
		virtual void createInsertField(
				unsigned int & fieldNo, std::ostream & insert, const std::string & name, const HookCommon * h) const;
		virtual void bindObjectAndExecute(ModelPartParam, DB::ModifyCommand *) const;
		virtual void bindObjectAndExecuteField(
				unsigned int & paramNo, DB::ModifyCommand *, ModelPartParam, const HookCommon *) const;

		DB::Connection * const connection;
		const std::string tableName;
	};

	class DLL_PUBLIC SqlAutoIdInsertSerializer : public SqlInsertSerializer {
	public:
		using SqlInsertSerializer::SqlInsertSerializer;

	protected:
		void createInsertField(unsigned int & fieldNo, std::ostream & insert, const std::string & name,
				const HookCommon * h) const override;
		void bindObjectAndExecuteField(
				unsigned int & paramNo, DB::ModifyCommand *, ModelPartParam, const HookCommon *) const override;
	};

	class DLL_PUBLIC SqlFetchIdInsertSerializer : public SqlAutoIdInsertSerializer {
	public:
		using SqlAutoIdInsertSerializer::SqlAutoIdInsertSerializer;

	protected:
		void bindObjectAndExecute(ModelPartParam, DB::ModifyCommand *) const override;
	};
}
