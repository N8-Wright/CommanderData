module;
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
module HashValue;

namespace Crypto
{
	HashValue::HashValue(const std::vector<uint8_t>& hash)
		: m_hash(std::move(hash))
	{
	}

	std::wstring HashValue::GetString() const
	{
		std::wostringstream oss;
		oss.fill('0');
		oss << std::hex;
		for (const auto& b : m_hash)
		{
			oss << std::setw(2) << static_cast<const unsigned int>(b);
		}

		return oss.str();
	}

	const std::vector<uint8_t>& HashValue::GetValue() const noexcept
	{
		return m_hash;
	}
}