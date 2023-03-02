#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <string>

namespace Util
{
    /// <summary>
    /// Gets the current directory of the running program.
    /// </summary>
    /// <returns>A string representing the current directory path.</returns>
    std::wstring CurrentDirectory()
    {
        std::wstring directory;
        const auto requiredBufferLength = GetCurrentDirectoryW(0, NULL);
        directory.reserve(requiredBufferLength);

        const auto pathLength = GetCurrentDirectoryW(requiredBufferLength, &directory[0]);
        if (pathLength == 0)
        {
            throw std::exception("Unable to get current directory");
        }

        return directory;
    }
}

int main()
{
    const auto directory = Util::CurrentDirectory();
    std::wcout << directory << std::endl;
}

