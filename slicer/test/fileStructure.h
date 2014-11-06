#ifndef SLICER_TEST_FILESTRUCTURE
#define SLICER_TEST_FILESTRUCTURE

#include <boost/filesystem/path.hpp>

namespace fs = boost::filesystem;

class FileStructure {
	public:
		FileStructure();
		~FileStructure();

	protected:
		const fs::path me;
		const fs::path base;
		const fs::path bjamout;
		const fs::path root;
		const fs::path slice;
		const fs::path tmp;
		const fs::path tmpf;
		const fs::path tmph;
};

#endif

