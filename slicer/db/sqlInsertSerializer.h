#ifndef SLICER_DB_SQLINSERTSERIALIZER_H
#define SLICER_DB_SQLINSERTSERIALIZER_H

#include <slicer/serializer.h>
#include <connection.h>
#include <visibility.h>
#include <buffer.h>

namespace Slicer {
	class DLL_PUBLIC SqlInsertSerializer : public Slicer::Serializer {
		public:
			typedef boost::shared_ptr<DB::ModifyCommand> ModifyPtr;

			SqlInsertSerializer(const DB::Connection *, const std::string & tableName);

			virtual void Serialize(Slicer::ModelPartPtr) override;

		protected:
			void SerializeObject(Slicer::ModelPartPtr) const;
			void SerializeSequence(Slicer::ModelPartPtr) const;
			ModifyPtr createInsert(Slicer::ModelPartPtr) const;
			virtual void createInsertField(int & fieldNo, AdHoc::Buffer & insert, const std::string & name, HookCommonPtr h) const;
			virtual void bindObjectAndExecute(Slicer::ModelPartPtr, DB::ModifyCommand *) const;
			virtual void bindObjectAndExecuteField(int & paramNo, DB::ModifyCommand *, Slicer::ModelPartPtr, HookCommonPtr) const;

			const DB::Connection * connection;
			const std::string tableName;
	};

	class DLL_PUBLIC SqlAutoIdInsertSerializer : public SqlInsertSerializer {
		public:
			template <typename ... P>
			SqlAutoIdInsertSerializer(const P & ... p) : SqlInsertSerializer(p...) { }

		protected:
			virtual void createInsertField(int & fieldNo, AdHoc::Buffer & insert, const std::string & name, HookCommonPtr h) const;
			virtual void bindObjectAndExecuteField(int & paramNo, DB::ModifyCommand *, Slicer::ModelPartPtr, HookCommonPtr) const;
	};

	class DLL_PUBLIC SqlFetchIdInsertSerializer : public SqlAutoIdInsertSerializer {
		public:
			template <typename ... P>
			SqlFetchIdInsertSerializer(const P & ... p) : SqlAutoIdInsertSerializer(p...) { }

		protected:
			virtual void bindObjectAndExecute(Slicer::ModelPartPtr, DB::ModifyCommand *) const;
	};
}

#endif

