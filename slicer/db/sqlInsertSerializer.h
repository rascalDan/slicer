#ifndef SLICER_DB_SQLINSERTSERIALIZER_H
#define SLICER_DB_SQLINSERTSERIALIZER_H

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

		void Serialize(Slicer::ModelPartForRootPtr) override;

	protected:
		void SerializeObject(const Slicer::ModelPartPtr &) const;
		void SerializeSequence(const Slicer::ModelPartPtr &) const;
		DB::ModifyCommandPtr createInsert(const Slicer::ModelPartPtr &) const;
		virtual void createInsertField(
				unsigned int & fieldNo, std::ostream & insert, const std::string & name, const HookCommon * h) const;
		virtual void bindObjectAndExecute(const Slicer::ModelPartPtr &, DB::ModifyCommand *) const;
		virtual void bindObjectAndExecuteField(
				unsigned int & paramNo, DB::ModifyCommand *, const Slicer::ModelPartPtr &, const HookCommon *) const;

		DB::Connection * const connection;
		const std::string tableName;
	};

	class DLL_PUBLIC SqlAutoIdInsertSerializer : public SqlInsertSerializer {
	public:
		template<typename... P>
		explicit SqlAutoIdInsertSerializer(P &&... p) : SqlInsertSerializer(std::forward<P>(p)...)
		{
		}

	protected:
		virtual void createInsertField(unsigned int & fieldNo, std::ostream & insert, const std::string & name,
				const HookCommon * h) const override;
		virtual void bindObjectAndExecuteField(unsigned int & paramNo, DB::ModifyCommand *,
				const Slicer::ModelPartPtr &, const HookCommon *) const override;
	};

	class DLL_PUBLIC SqlFetchIdInsertSerializer : public SqlAutoIdInsertSerializer {
	public:
		template<typename... P>
		explicit SqlFetchIdInsertSerializer(P &&... p) : SqlAutoIdInsertSerializer(std::forward<P>(p)...)
		{
		}

	protected:
		virtual void bindObjectAndExecute(const Slicer::ModelPartPtr &, DB::ModifyCommand *) const override;
	};
}

#endif
