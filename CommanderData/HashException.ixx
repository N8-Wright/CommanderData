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
		HashException(const char* what);
		HashException(const std::string& what);
		virtual const char* what() const noexcept override;
	private:
		std::string m_what;
	};
}