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
        for (const auto& findData : Filesystem::IterateDirectory())
        {
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                std::wcout << findData.cFileName << L"\t<DIR>\n";
            }
            else
            {
                LARGE_INTEGER filesize{};
                filesize.LowPart = findData.nFileSizeLow;
                filesize.HighPart = findData.nFileSizeHigh;

                std::wcout << findData.cFileName << L"  " << filesize.QuadPart << L"\tbytes\n";
            }
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what();
    }
}

