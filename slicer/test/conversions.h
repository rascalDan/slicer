#ifndef SLICER_TEST_CONVERSIONS_H
#define SLICER_TEST_CONVERSIONS_H

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <visibility.h>
#include <slicer/modelPartsTypes.h>
#include <types.h>

namespace TestModule {
	DLL_PUBLIC extern int completions;

	class DLL_PUBLIC AbValidator : public Slicer::ModelPartForClass<ClassType> {
		public:
			AbValidator(ClassTypePtr &);

			void Complete() override;
	};

	class DLL_PUBLIC MonthValidator : public Slicer::ModelPartForSimple<::Ice::Short> {
		public:
			MonthValidator(::Ice::Short &);

			void Complete() override;
	};
}
namespace Slicer {
	DLL_PUBLIC
	::TestModule::DateTime
	stringToDateTime(const std::string & in);
	DLL_PUBLIC
	std::string
	dateTimeToString(const ::TestModule::DateTime & in);
}

#endif

