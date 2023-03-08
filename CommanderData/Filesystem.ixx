module;

#include <string>
#include <boost/coroutine2/coroutine.hpp>
#include <boost/exception/exception.hpp>
#include <Windows.h>
#include <vector>
#include <fmt/core.h>

export module Filesystem;
export import HashValue;

namespace Filesystem
{
	export typedef boost::error_info<struct FilePathErrorInfo, std::wstring> FilePathInfo;
	export typedef boost::error_info<struct FileNameErrorInfo, std::wstring> FileNameInfo;

	export struct FilesystemException : virtual boost::exception, virtual std::exception {
	public:
		FilesystemException(const char *what)
		{
			DWORD error = ::GetLastError();
			std::string message = std::system_category().message(error);
			m_what = fmt::format("{}. {}", what, message);
		}

		FilesystemException(std::string what)
		{
			DWORD error = ::GetLastError();
			std::string message = std::system_category().message(error);
			m_what = fmt::format("{}. {}", what, message);
		}

		virtual const char* what() const noexcept override
		{
			return m_what.c_str();
		}
	private:
		std::string m_what;
	};

	export struct FileNotFoundException : public FilesystemException {
	public:
		FileNotFoundException(const char* what)
			: FilesystemException(what)
		{}
	};

    export std::wstring CurrentDir();
	export boost::coroutines2::coroutine<WIN32_FIND_DATA>::pull_type IterateDirectory(const std::wstring& dir = CurrentDir() + L"\\*");
	export boost::coroutines2::coroutine<WIN32_FIND_STREAM_DATA>::pull_type IterateStreams(const WCHAR* file);
	export Crypto::HashValue HashFileContents(std::wstring file);
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

	export template <typename T>
	void WriteFile(std::wstring_view file, T content)
	{
		WriteFile(file, content.data(), content.size());
	}
}