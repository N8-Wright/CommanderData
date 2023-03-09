module;
#include <string>
#include <Windows.h>
#include <boost/exception/all.hpp>
#include <fmt/core.h>
export module HashException;

namespace Crypto
{
	export struct HashException : virtual boost::exception, virtual std::exception {
	public:
		HashException(const char* what)
		{
			DWORD error = ::GetLastError();
			std::string message = std::system_category().message(error);
			m_what = fmt::format("{}. {}", what, message);
		}

		HashException(const std::string& what)
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