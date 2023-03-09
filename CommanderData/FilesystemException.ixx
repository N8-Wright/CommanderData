module;
#include <boost/exception/all.hpp>
#include <fmt/core.h>
#include <Windows.h>
export module FilesystemException;

namespace Filesystem
{
	export typedef boost::error_info<struct FilePathErrorInfo, std::wstring> FilePathInfo;
	export typedef boost::error_info<struct FileNameErrorInfo, std::wstring> FileNameInfo;

	export struct FilesystemException : virtual boost::exception, virtual std::exception {
	public:
		FilesystemException(const char* what)
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
}