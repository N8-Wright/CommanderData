#include <iostream>
#include <string>

#include <Windows.h>
#include <fileapi.h>
#include <handleapi.h>
#include <boost/exception/all.hpp>

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

                const auto hash = Filesystem::HashFileContents(findData.cFileName);
                std::wcout << findData.cFileName << L"\t" << hash << std::endl;

                for (const auto& streamData : Filesystem::IterateStreams(findData.cFileName))
                {
                    std::wcout << L"\t" << streamData.cStreamName << L"\t" << streamData.StreamSize.QuadPart << L" bytes\n";
                }
            }
        }
    }
    catch (Filesystem::FilesystemException& e)
    {
        std::cerr << boost::diagnostic_information(e) << std::endl;
        std::cerr << e.what();
        if (const auto fileName = boost::get_error_info<Filesystem::FileNameInfo>(e))
        {
            std::wcerr << L" '" << fileName->c_str() << L"'";
        }
        if (const auto path = boost::get_error_info<Filesystem::FilePathInfo>(e))
        {
            std::wcerr << L", path: '" << path->c_str() << L"'";
        }

        std::cerr << std::endl;
    }
}

