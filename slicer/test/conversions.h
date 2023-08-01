#ifndef SLICER_TEST_CONVERSIONS_H
#define SLICER_TEST_CONVERSIONS_H

#include "classtype.h"
#include "structs.h"
#include <Ice/Config.h>
#include <Ice/Optional.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <optional>
#include <slicer/modelPartsTypes.h>
#include <string>
#include <visibility.h>

namespace TestModule {
	DLL_PUBLIC extern int completions;

	class DLL_PUBLIC AbValidator : public Slicer::ModelPartForClass<ClassType> {
	public:
		explicit AbValidator(ClassTypePtr *);

		void Complete() override;
	};

	class DLL_PUBLIC MonthValidator : public Slicer::ModelPartForSimple<::Ice::Short> {
	public:
		explicit MonthValidator(::Ice::Short *);

		void Complete() override;
	};
}

namespace Slicer {
	DLL_PUBLIC
	::TestModule::DateTime stringToDateTime(const std::string & in);
	DLL_PUBLIC
	std::string dateTimeToString(const ::TestModule::DateTime & in);
	DLL_PUBLIC
	Ice::optional<Ice::Int> str2int(const std::string &);
	DLL_PUBLIC
	std::string int2str(const Ice::optional<Ice::Int> &);
	DLL_PUBLIC
	std::string ptimeToString(const boost::posix_time::ptime &);
	DLL_PUBLIC
	boost::posix_time::ptime stringToPtime(const std::string &);
}

#endif
