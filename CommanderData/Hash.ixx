module;

#include <span>
#include <vector>
#include <Windows.h>
#include <boost/exception/all.hpp>
#include <fmt/core.h>

export module Hash;
export import HashValue;
export import HashException;

namespace Crypto
{
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
		HashValue GetResult() const;
	private:
		HCRYPTPROV m_cryptoProvider;
		HCRYPTPROV m_hashHandle;
	};
}