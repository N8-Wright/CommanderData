#include <iostream>
#include <string>

#include <Windows.h>
#include <fileapi.h>
#include <handleapi.h>
#include <boost/exception/all.hpp>
#include <shlwapi.h>
#include <fmt/xchar.h>


#include <boost/program_options.hpp>

import Filesystem;
struct CheckStats
{
    std::vector <std::wstring> fileHashesFailed;
    size_t directoriesProcessed;
    size_t filesProcessed;
};

void CheckDirectory(CheckStats& stats, const std::wstring& dir = Filesystem::CurrentDir())
{
    for (const auto& dirEntry : Filesystem::IterateDirectory(dir + L"\\*"))
    {
        try
        {
            const auto relativeEntryName = std::wstring(dirEntry.cFileName);
            const auto entryName = fmt::format(L"{}\\{}", dir, dirEntry.cFileName);

            if (dirEntry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (relativeEntryName == L".")
                {
                    continue;
                }
                else if (relativeEntryName == L"..")
                {
                    continue;
                }
                else
                {
                    CheckDirectory(stats, entryName);
                }
            }
            else
            {
                LARGE_INTEGER filesize{};
                filesize.LowPart = dirEntry.nFileSizeLow;
                filesize.HighPart = dirEntry.nFileSizeHigh;

                const auto lastWriteTimeStream = fmt::format(L"{}:CommanderDataLastWriteTime", entryName);
                const auto hashStream = fmt::format(L"{}:CommanderDataHash", entryName);

                FILETIME previousWriteTime{};
                try
                {
                    previousWriteTime = Filesystem::ReadFile<FILETIME>(lastWriteTimeStream);
                }
                catch (Filesystem::FileNotFoundException&)
                {
                }

                const auto hash = Filesystem::HashFileContents(entryName);

                // NOTE: This comparison will be true if our previous write time was not able
                // to be read from the last write time stream since, presumably, a failure to
                // read means that it's not there.
                if (previousWriteTime.dwLowDateTime != dirEntry.ftLastWriteTime.dwLowDateTime ||
                    previousWriteTime.dwHighDateTime != dirEntry.ftLastWriteTime.dwHighDateTime)
                {
                    Filesystem::WriteFile(lastWriteTimeStream, &dirEntry.ftLastWriteTime, sizeof(dirEntry.ftLastWriteTime));
                    Filesystem::WriteFile(hashStream, hash.GetValue());

                    // Since writes to alternate data streams affect the last modified time of the file,
                    // we should set it back to the way we found it so that what's written in our stream
                    // is actually comparable with the file's metatada.
                    Filesystem::SetFileTime(entryName, dirEntry.ftLastWriteTime);

                    const auto hashStr = hash.GetString();
                    std::wcout << L"'" << relativeEntryName << L"' was updated since last run. Writing new hash '" << hashStr << L"'\n";
                }
                else
                {
                    const auto previousHash = Crypto::HashValue(Filesystem::ReadFile(hashStream));
                    if (previousHash.GetValue() != hash.GetValue())
                    {
                        std::wcerr << L"File hashes do not match for '" << entryName << L"'. Expected '"
                            << hash.GetString() << L"' but found '"
                            << hash.GetString() << L"'\n";

                        stats.fileHashesFailed.push_back(entryName);
                    }
                    else
                    {
                        std::wcout << L"Hashes match for '" << entryName << L"'\n";
                    }
                }

                stats.filesProcessed++;
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

    stats.directoriesProcessed++;
}

void Cleanup(const std::wstring& dir = Filesystem::CurrentDir())
{
    std::wcout << "Cleaning up '" << dir << "'...\n";
    for (const auto& dirEntry : Filesystem::IterateDirectory(dir + L"\\*"))
    {
        try
        {
            const auto relativeEntryName = std::wstring(dirEntry.cFileName);
            const auto entryName = fmt::format(L"{}\\{}", dir, dirEntry.cFileName);

            if (dirEntry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (relativeEntryName == L".")
                {
                    continue;
                }
                else if (relativeEntryName == L"..")
                {
                    continue;
                }
                else
                {
                    Cleanup(entryName);
                }
            }
            else
            {
                const auto lastWriteTimeStream = fmt::format(L"{}:CommanderDataLastWriteTime", entryName);
                const auto hashStream = fmt::format(L"{}:CommanderDataHash", entryName);

                try
                {
                    Filesystem::DelFile(hashStream);
                }
                catch (Filesystem::FilesystemException&)
                { }

                try
                {
                    Filesystem::DelFile(lastWriteTimeStream);
                }
                catch (Filesystem::FilesystemException&)
                { }
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
}

int main(int argc, char *argv[])
{
    std::cout << "Commander Data V0.01\n";
    try
    {
        boost::program_options::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("clean", "cleanup any existing Commander Data alternate data streams")
            ("include-path,I", boost::program_options::wvalue<std::vector<std::wstring>>(), "include path");

        boost::program_options::variables_map variableMap;
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), variableMap);
        boost::program_options::notify(variableMap);

        if (variableMap.count("help"))
        {
            std::cout << desc << "\n";
        }
        else if (variableMap.count("clean"))
        {
            Cleanup();
        }
        else
        {
            std::vector<std::wstring> paths = { Filesystem::CurrentDir() };
            if (variableMap.count("include-path"))
            {
                const auto& extraIncludePaths = variableMap["include-path"].as<std::vector<std::wstring>>();
                paths.insert(paths.end(), extraIncludePaths.begin(), extraIncludePaths.end());
            }

            CheckStats stats{};
            for (const auto& dir : paths)
            {
                CheckDirectory(stats);
            }

            std::wcout << "Finished processing " << stats.filesProcessed << " files in " << stats.directoriesProcessed << " directories.\n";
            if (stats.fileHashesFailed.size() > 0)
            {
                std::cerr << "Failed hashes for " << stats.fileHashesFailed.size()
                    << ". This may indicate file corruption for the following files: \n";
                for (const auto& failedHash : stats.fileHashesFailed)
                {
                    std::wcerr << failedHash << L"\n";
                }
            }
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Unexpected error encountered. '" << e.what() << "'\n";
        return 1;
    }

    return 0;
}

