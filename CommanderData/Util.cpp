module;

#include <string>
#include <vector>
#include <Windows.h>
#include <sstream>
#include <iomanip>

module Util;

namespace Util
{
	std::wstring GetStringRepresentationOfHash(const std::vector<BYTE>& hash)
	{
        std::wostringstream oss;
        oss.fill('0');
        oss << std::hex;
        for (const auto& b : hash)
        {
            oss << std::setw(2) << static_cast<const unsigned int>(b);
        }

        return oss.str();
	}
}