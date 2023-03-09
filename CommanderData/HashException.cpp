module;
#include <Windows.h>
#include <string>
#include <system_error>
#include <fmt/core.h>
#include <boost/exception/all.hpp>
module HashException;

namespace Crypto
{
	HashException::HashException(const char* what)
	{
		DWORD error = ::GetLastError();
		std::string message = std::system_category().message(error);
		m_what = fmt::format("{}. {}", what, message);
	}

	HashException::HashException(const std::string& what)
	{
		DWORD error = ::GetLastError();
		std::string message = std::system_category().message(error);
		m_what = fmt::format("{}. {}", what, message);
	}

	const char* HashException::what() const noexcept
	{
		return m_what.c_str();
	}
}