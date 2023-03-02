#include <iostream>
#include <string>

#include <Windows.h>
#include <fileapi.h>
#include <handleapi.h>

import Filesystem;

int main()
{
    try
    {
        auto directory = Filesystem::CurrentDir();
        directory.append(L"\\.*");

        std::wcout << directory << std::endl;
        WIN32_FIND_DATA findData;
        auto fileHandle = FindFirstFileW(directory.c_str(), &findData);
        if (fileHandle == INVALID_HANDLE_VALUE)
        {
            throw std::exception("Unable to iterate directory");
        }
        do
        {
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                std::wcout << findData.cFileName << L"  <DIR>\n";
            }
            else
            {
                LARGE_INTEGER filesize{};
                filesize.LowPart = findData.nFileSizeLow;
                filesize.HighPart = findData.nFileSizeHigh;

                std::wcout << findData.cFileName << L"  " << filesize.QuadPart << L" bytes\n";
            }
        } while (FindNextFile(fileHandle, &findData) != 0);

        const auto dwError = GetLastError();
        if (dwError != ERROR_NO_MORE_FILES)
        {
            throw std::exception("Unable to finish iterating directory");
        }

        FindClose(fileHandle);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what();
    }
}

