#ifndef SLICER_TEST_FILESTRUCTURE
#define SLICER_TEST_FILESTRUCTURE

#include <boost/filesystem/path.hpp>
#include <visibility.h>

namespace fs = boost::filesystem;

class DLL_PUBLIC FileStructure {
	public:
		FileStructure();
		~FileStructure();

	protected:
		const fs::path & me;
		const fs::path base;
		const fs::path bjamout;
		const fs::path & root;
		const fs::path included;
		const fs::path slice;
		const fs::path tmp;
};

#endif

