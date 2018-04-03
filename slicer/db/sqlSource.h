#ifndef SLICER_DB_SQLSOURCE_H
#define SLICER_DB_SQLSOURCE_H

#include <slicer/modelParts.h>
#include <column.h>
#include <boost/date_time/posix_time/ptime.hpp>

namespace Slicer {
	class SqlSource : public Slicer::ValueSource,
			public Slicer::TValueSource<boost::posix_time::time_duration>,
			public Slicer::TValueSource<boost::posix_time::ptime>
	{
		public:
			SqlSource(const DB::Column & c);

			bool isNull() const;
			void set(boost::posix_time::ptime & b) const override;
			void set(boost::posix_time::time_duration & b) const override;
			void set(bool & b) const override;
			void set(Ice::Byte & b) const override;
			void set(Ice::Short & b) const override;
			void set(Ice::Int & b) const override;
			void set(Ice::Long & b) const override;
			void set(Ice::Float & b) const override;
			void set(Ice::Double & b) const override;
			void set(std::string & b) const override;

		private:
			const DB::Column & column;
	};
	typedef std::shared_ptr<SqlSource> SqlSourcePtr;
}

#endif

