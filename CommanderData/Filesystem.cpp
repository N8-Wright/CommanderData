module;

#include <string>
#include <Windows.h>

module Filesystem;
namespace Filesystem
{
    std::wstring CurrentDir()
    {
        const auto requiredBufferLength = GetCurrentDirectoryW(0, NULL);
        std::wstring directory;
        directory.reserve(requiredBufferLength);
        directory.resize(requiredBufferLength - 1);
        const auto pathLength = GetCurrentDirectoryW(requiredBufferLength, directory.data());
        if (pathLength == 0)
        {
            throw std::exception("Unable to get current directory");
        }

        return directory;
    }
}