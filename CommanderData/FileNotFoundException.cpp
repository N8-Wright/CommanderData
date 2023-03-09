module;
#include <boost/exception/all.hpp>
module FileNotFoundException;

namespace Filesystem
{
	FileNotFoundException::FileNotFoundException(const char* what)
		: FilesystemException(what)
	{}
}