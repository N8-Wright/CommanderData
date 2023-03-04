module;

#include <string>
#include <vector>
#include <Windows.h>

export module Util;

namespace Util
{
	export std::wstring GetStringRepresentationOfHash(const std::vector<BYTE> &hash);
}
