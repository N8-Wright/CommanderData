module;

#include <string>
#include <boost/coroutine2/coroutine.hpp>
#include <boost/exception/exception.hpp>
#include <Windows.h>
#include <vector>

export module Filesystem;

namespace Filesystem
{
	export typedef boost::error_info<struct FilePathErrorInfo, std::wstring> FilePathInfo;
	export typedef boost::error_info<struct FileNameErrorInfo, std::wstring> FileNameInfo;
	export typedef boost::error_info<struct FileNameErrorInfo, std::wstring_view> FileNameInfo2;

	export struct FilesystemException : virtual boost::exception, virtual std::exception {
	public:
		FilesystemException(const char *what)
			: m_what(what)
		{}

		virtual const char* what() const noexcept override
		{
			return m_what.c_str();
		}
	private:
		std::string m_what;
	};

    export std::wstring CurrentDir();
	export boost::coroutines2::coroutine<WIN32_FIND_DATA>::pull_type IterateDirectory(const std::wstring& dir = CurrentDir() + L"\\*");
	export boost::coroutines2::coroutine<WIN32_FIND_STREAM_DATA>::pull_type IterateStreams(const WCHAR* file);
	export std::vector<BYTE> HashFileContents(std::wstring file);
	export std::wstring GetSHA256(const WCHAR* file);

	export void WriteFile(std::wstring_view file, const void* data, size_t size);

	export template <typename T>
	void WriteFile(std::wstring_view file, T content)
	{
		WriteFile(file, content.data(), content.size());
	}
}