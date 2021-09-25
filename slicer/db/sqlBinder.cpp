#include "sqlBinder.h"
#include <command.h>

namespace Slicer {
	SqlBinder::SqlBinder(DB::Command & c, unsigned int i) : command(c), idx(i) { }

	void
	SqlBinder::get(const boost::posix_time::ptime & b) const
	{
		command.bindParamT(idx, b);
	}

	void
	SqlBinder::get(const boost::posix_time::time_duration & b) const
	{
		command.bindParamT(idx, b);
	}

	void
	SqlBinder::get(const bool & b) const
	{
		command.bindParamB(idx, b);
	}

	void
	SqlBinder::get(const Ice::Byte & b) const
	{
		command.bindParamI(idx, b);
	}

	void
	SqlBinder::get(const Ice::Short & b) const
	{
		command.bindParamI(idx, b);
	}

	void
	SqlBinder::get(const Ice::Int & b) const
	{
		command.bindParamI(idx, b);
	}

	void
	SqlBinder::get(const Ice::Long & b) const
	{
		command.bindParamI(idx, b);
	}

	void
	SqlBinder::get(const Ice::Float & b) const
	{
		command.bindParamF(idx, b);
	}

	void
	SqlBinder::get(const Ice::Double & b) const
	{
		command.bindParamF(idx, b);
	}

	void
	SqlBinder::get(const std::string & b) const
	{
		command.bindParamS(idx, b);
	}
}
