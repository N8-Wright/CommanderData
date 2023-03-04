#include <iostream>
#include <string>

#include <Windows.h>
#include <fileapi.h>
#include <handleapi.h>
#include <boost/exception/all.hpp>
#include <shlwapi.h>
#include <fmt/xchar.h>

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
                for (const auto& streamData : Filesystem::IterateStreams(findData.cFileName))
                {
                    std::wcout << L"\t" << streamData.cStreamName << L"\t" << streamData.StreamSize.QuadPart << L" bytes\n";
                }

                const auto lastWriteTimeStream = fmt::format(L"{}:CommanderDataLastWriteTime", findData.cFileName);
                const auto hashStream = fmt::format(L"{}:CommanderDataHash", findData.cFileName);

                FILETIME previousWriteTime{};
                try
                {
                    previousWriteTime = Filesystem::ReadFile<FILETIME>(lastWriteTimeStream);
                }
                catch (Filesystem::FileNotFoundException&)
                {
                }

                if (previousWriteTime.dwLowDateTime != findData.ftLastWriteTime.dwLowDateTime &&
                    previousWriteTime.dwHighDateTime != findData.ftLastWriteTime.dwHighDateTime)
                {
                    Filesystem::WriteFile(fmt::format(L"{}:CommanderDataLastWriteTime", findData.cFileName), &findData.ftLastWriteTime, sizeof(findData.ftLastWriteTime));
                    Filesystem::WriteFile(fmt::format(L"{}:CommanderDataHash", findData.cFileName), hash);
                }
                else
                {
                    const auto previousHash = Filesystem::ReadFile(hashStream);
                    if (previousHash != hash)
                    {
                        BOOST_THROW_EXCEPTION(std::exception("File corruption!!!!"));
                    }
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

