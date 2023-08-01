#include "sqlCommon.h"
#include <compileTimeFormatter.h>
#include <slicer/modelParts.h>
#include <sqlExceptions.h>
#include <string_view>

namespace Slicer {
	constexpr std::string_view md_pkey {"db:pkey"};
	constexpr std::string_view md_auto {"db:auto"};
	constexpr std::string_view md_ignore {"db:ignore"};
	constexpr std::string_view md_global_ignore {"ignore"};

	bool
	isPKey(const HookCommon * h)
	{
		return h->GetMetadata().flagSet(md_pkey) && isBind(h);
	}

	bool
	isAuto(const HookCommon * h)
	{
		return h->GetMetadata().flagSet(md_auto) && isBind(h);
	}

	bool
	isNotAuto(const HookCommon * h)
	{
		return h->GetMetadata().flagNotSet(md_auto) && isBind(h);
	}

	bool
	isBind(const HookCommon * h)
	{
		return h->GetMetadata().flagNotSet(md_global_ignore) && h->GetMetadata().flagNotSet(md_ignore);
	}

	bool
	isValue(const HookCommon * h)
	{
		return h->GetMetadata().flagNotSet(md_auto) && h->GetMetadata().flagNotSet(md_pkey) && isBind(h);
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
