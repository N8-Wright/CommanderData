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

	ALG_ID GetWin32Algorithm(HashAlgorithm algorithm)
	{
		switch (algorithm)
		{
		case HashAlgorithm::Sha256:
			return CALG_SHA_256;
		default:
			BOOST_THROW_EXCEPTION(std::invalid_argument("Invalid hash algorithm"));
		}
	}

	export class Hash
	{
	public:
		Hash(HashAlgorithm algorithm)
		{
			if (!CryptAcquireContextW(&m_cryptoProvider,
				NULL,
				NULL,
				PROV_RSA_AES,
				CRYPT_VERIFYCONTEXT))
			{
				BOOST_THROW_EXCEPTION(HashException("CryptAcquireContext failed"));
			}

			if (!CryptCreateHash(m_cryptoProvider, GetWin32Algorithm(algorithm), 0, 0, &m_hashHandle))
			{
				CryptReleaseContext(m_cryptoProvider, 0);
				BOOST_THROW_EXCEPTION(HashException("CryptCreateHash failed"));
			}
		}

		~Hash()
		{
			CryptReleaseContext(m_cryptoProvider, 0);
			CryptDestroyHash(m_hashHandle);
		}

		void HashData(std::span<uint8_t> bytes) const
		{
			if (bytes.size() > MAXDWORD)
			{
				BOOST_THROW_EXCEPTION(HashException("Unable to hash more than MAXDWORD at once"));
			}

			if (!CryptHashData(m_hashHandle, bytes.data(), static_cast<DWORD>(bytes.size()), 0))
			{
				BOOST_THROW_EXCEPTION(HashException("CtyptHashData failed"));
			}
		}

		HashValue GetResult() const
		{
			DWORD hashSize = 0;
			DWORD count = sizeof(DWORD);
			if (!CryptGetHashParam(m_hashHandle, HP_HASHSIZE, reinterpret_cast<BYTE*>(&hashSize), &count, 0))
			{
				BOOST_THROW_EXCEPTION(HashException("CryptGetHashParam HP_HASHSIZE failed"));
			}

			std::vector<uint8_t> hashOutputBuffer(hashSize);
			if (!CryptGetHashParam(m_hashHandle, HP_HASHVAL, hashOutputBuffer.data(), &hashSize, 0))
			{
				BOOST_THROW_EXCEPTION(HashException("CryptGetHashParam HP_HASHVAL failed"));
			}

			return HashValue(hashOutputBuffer);
		}

	private:
		HCRYPTPROV m_cryptoProvider;
		HCRYPTPROV m_hashHandle;
	};
}