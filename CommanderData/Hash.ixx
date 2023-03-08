module;

#include <span>
#include <vector>
#include <Windows.h>
#include <boost/exception/all.hpp>
#include <fmt/core.h>

export module Hash;

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

	export enum class HashAlgorithm
	{
		Sha256,
	};

	export class Hash
	{
	public:
		Hash(HashAlgorithm algorithm);
		~Hash();
		void HashData(std::span<uint8_t> bytes) const;
		std::vector<uint8_t> GetResult() const;
	private:
		HCRYPTPROV m_cryptoProvider;
		HCRYPTPROV m_hashHandle;
	};
}