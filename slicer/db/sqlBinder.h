#ifndef SLICER_DB_SQLBINDER_H
#define SLICER_DB_SQLBINDER_H

#include <boost/date_time/posix_time/ptime.hpp>
#include <command.h>
#include <slicer/modelParts.h>

namespace Slicer {
	class SqlBinder :
		public Slicer::ValueTarget,
		public Slicer::TValueTarget<boost::posix_time::time_duration>,
		public Slicer::TValueTarget<boost::posix_time::ptime> {
	public:
		SqlBinder(DB::Command & c, unsigned int idx);

		void get(const boost::posix_time::ptime & b) const override;
		void get(const boost::posix_time::time_duration & b) const override;
		void get(const bool & b) const override;
		void get(const Ice::Byte & b) const override;
		void get(const Ice::Short & b) const override;
		void get(const Ice::Int & b) const override;
		void get(const Ice::Long & b) const override;
		void get(const Ice::Float & b) const override;
		void get(const Ice::Double & b) const override;
		void get(const std::string & b) const override;

	private:
		DB::Command & command;
		const unsigned int idx;
	};
	typedef std::shared_ptr<SqlBinder> SqlBinderPtr;
}

#endif
