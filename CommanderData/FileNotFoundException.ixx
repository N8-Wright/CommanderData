module;
#include <boost/exception/all.hpp>
export module FileNotFoundException;
import FilesystemException;

namespace Filesystem
{
	export struct FileNotFoundException : public FilesystemException {
	public:
		FileNotFoundException(const char* what);
	};
}