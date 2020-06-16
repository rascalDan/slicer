#include "sqlCommon.h"
#include <compileTimeFormatter.h>
#include <metadata.h>
#include <sqlExceptions.h>

namespace Slicer {
	const std::string md_pkey = "db:pkey";
	const std::string md_auto = "db:auto";
	const std::string md_ignore = "db:ignore";
	const std::string md_global_ignore = "ignore";

	bool
	isPKey(const HookCommon * h)
	{
		return metaDataFlagSet(h->GetMetadata(), md_pkey) && isBind(h);
	}

	bool
	isAuto(const HookCommon * h)
	{
		return metaDataFlagSet(h->GetMetadata(), md_auto) && isBind(h);
	}

	bool
	isNotAuto(const HookCommon * h)
	{
		return metaDataFlagNotSet(h->GetMetadata(), md_auto) && isBind(h);
	}

	bool
	isBind(const HookCommon * h)
	{
		return metaDataFlagNotSet(h->GetMetadata(), md_global_ignore)
				&& metaDataFlagNotSet(h->GetMetadata(), md_ignore);
	}

	bool
	isValue(const HookCommon * h)
	{
		return metaDataFlagNotSet(h->GetMetadata(), md_auto) && metaDataFlagNotSet(h->GetMetadata(), md_pkey)
				&& isBind(h);
	}

	void
	TooManyRowsReturned::ice_print(std::ostream & s) const
	{
		s << "Too many rows returned";
	}

	void
	NoRowsReturned::ice_print(std::ostream & s) const
	{
		s << "No rows returned";
	}

	void
	NoRowsFound::ice_print(std::ostream & s) const
	{
		s << "No rows found";
	}

	AdHocFormatter(UnsuitableIdFieldTypeMsg, "Unsuitable id field type [%?]");
	void
	UnsuitableIdFieldType::ice_print(std::ostream & s) const
	{
		UnsuitableIdFieldTypeMsg::write(s, type);
	}

}
