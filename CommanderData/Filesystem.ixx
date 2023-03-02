module;

#include <string>
#include <boost/coroutine2/coroutine.hpp>
#include <Windows.h>

export module Filesystem;

namespace Filesystem
{
	export std::wstring CurrentDir();
	export boost::coroutines2::coroutine<WIN32_FIND_DATA>::pull_type IterateDirectory(const std::wstring& dir);
}