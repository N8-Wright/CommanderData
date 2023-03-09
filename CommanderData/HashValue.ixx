module;
#include <vector>
#include <string>
export module HashValue;
namespace Crypto
{
	export class HashValue
	{
	public:
		HashValue(const std::vector<uint8_t>& hash);
		std::wstring GetString() const;
		const std::vector<uint8_t>& GetValue() const noexcept;
	private:
		std::vector<uint8_t> m_hash;
	};
}