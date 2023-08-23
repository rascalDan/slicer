#ifndef SLICER_DB_SQLSOURCE_H
#define SLICER_DB_SQLSOURCE_H

#include "slicer/modelParts.h"
#include <Ice/Config.h>
#include <memory>
#include <string>

namespace DB {
	class Column;
}

namespace boost::posix_time {
	class ptime;
	class time_duration;
}

namespace Slicer {
	class SqlSource :
		public Slicer::ValueSource,
		public Slicer::TValueSource<boost::posix_time::time_duration>,
		public Slicer::TValueSource<boost::posix_time::ptime> {
	public:
		explicit SqlSource(const DB::Column & c);

		[[nodiscard]] bool isNull() const;
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
}

#endif
