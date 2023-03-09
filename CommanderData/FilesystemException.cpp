module;
#include <Windows.h>
#include <fmt/core.h>
#include <system_error>
#include <boost/exception/all.hpp>
module FilesystemException;

namespace Filesystem
{
	FilesystemException::FilesystemException(const char* what)
	{
		DWORD error = ::GetLastError();
		std::string message = std::system_category().message(error);
		m_what = fmt::format("{}. {}", what, message);
	}

	FilesystemException::FilesystemException(std::string what)
	{
		DWORD error = ::GetLastError();
		std::string message = std::system_category().message(error);
		m_what = fmt::format("{}. {}", what, message);
	}

	const char* FilesystemException::what() const noexcept
	{
		return m_what.c_str();
	}
}