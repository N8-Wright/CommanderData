module;
#include <string>
#include <span>
#include <vector>
#include <array>
#include <boost/coroutine2/coroutine.hpp>
#include <boost/exception/all.hpp>
#include <fmt/core.h>
#include <Windows.h>
#include <atlfile.h>
export module Filesystem;
export import Hash;
export import HashValue;
export import FilesystemException;
export import FileNotFoundException;
namespace Filesystem
{
	using namespace boost::coroutines2;
	export std::wstring CurrentDir();
	export coroutine<WIN32_FIND_DATA>::pull_type IterateDirectory(const std::wstring& dir = CurrentDir() + L"\\*");
	export coroutine<WIN32_FIND_STREAM_DATA>::pull_type IterateStreams(const WCHAR* file);
	export Crypto::HashValue HashFileContents(const std::wstring& file);
	export void SetFileTime(std::wstring_view file, FILETIME filetime);
	export std::vector<BYTE> ReadFile(std::wstring_view file);

	export template <typename T>
	T ReadFile(std::wstring_view file)
	{
		std::vector<BYTE> contents = Filesystem::ReadFile(file);
		T tocast{};
		memcpy(&tocast, contents.data(), contents.size());
		return tocast;
	}

	export void WriteFile(std::wstring_view file, const void* data, size_t size);
	export void WriteFile(std::wstring_view file, auto content)
	{
		WriteFile(file, content.data(), content.size());
	}
}