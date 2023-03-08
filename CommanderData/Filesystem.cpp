module;

#include <string>
#include <array>
#include <sstream>
#include <boost/coroutine2/coroutine.hpp>
#include <fmt/core.h>
#include <Windows.h>
#include <iomanip>
#include <boost/exception/all.hpp>
#include <atlfile.h>

module Filesystem;

import Hash;
import HashValue;

namespace Filesystem
{
    using namespace boost::coroutines2;
    std::wstring CurrentDir()
    {
        const auto requiredBufferLength = GetCurrentDirectoryW(0, NULL);
        std::wstring directory;
        directory.reserve(requiredBufferLength);
        directory.resize(requiredBufferLength - 1);
        const auto pathLength = GetCurrentDirectoryW(requiredBufferLength, directory.data());
        if (pathLength == 0)
        {
            BOOST_THROW_EXCEPTION(FilesystemException("Unable to get current directory"));
        }

        return directory;
    }

    coroutine<WIN32_FIND_DATA>::pull_type IterateDirectory(const std::wstring& dir)
    {
        return coroutine<WIN32_FIND_DATA>::pull_type(
            [&](coroutine<WIN32_FIND_DATA>::push_type& sink) {
                WIN32_FIND_DATA findData;
                auto findHandle = FindFirstFileW(dir.c_str(), &findData);
                if (findHandle == INVALID_HANDLE_VALUE)
                {
                    BOOST_THROW_EXCEPTION(FilesystemException("Unable to iterate directory") << FilePathInfo(dir));
                }
                do
                {
                    sink(findData);
                } while (FindNextFileW(findHandle, &findData) != 0);

                FindClose(findHandle);
                const auto dwError = GetLastError();
                if (dwError != ERROR_NO_MORE_FILES)
                {
                    BOOST_THROW_EXCEPTION(FilesystemException("Unable to finish iterating directory") << FilePathInfo(dir));
                }
            }
        );
    }

    coroutine<WIN32_FIND_STREAM_DATA>::pull_type IterateStreams(const WCHAR* file)
    {
        return coroutine<WIN32_FIND_STREAM_DATA>::pull_type(
            [file](coroutine<WIN32_FIND_STREAM_DATA>::push_type& sink)
            {
                WIN32_FIND_STREAM_DATA findData{};
                auto findHandle = FindFirstStreamW(file, FindStreamInfoStandard, &findData, 0);
                if (findHandle == INVALID_HANDLE_VALUE)
                {
                    BOOST_THROW_EXCEPTION(FilesystemException("Unable to iterate file streams") << FileNameInfo(file));
                }
                do
                {
                    sink(findData);
                } while (FindNextStreamW(findHandle, &findData) != 0);

                FindClose(findHandle);
                const auto lastError = GetLastError();
                if (lastError != ERROR_HANDLE_EOF)
                {
                    BOOST_THROW_EXCEPTION(FilesystemException("Unable to finish iterating file streams") << FileNameInfo(file));
                }
            }
        );
    }

    Crypto::HashValue HashFileContents(std::wstring file)
    {
        using namespace Crypto;
        CAtlFile fileHandle;
        if (S_OK != fileHandle.Create(
            file.data(),
            GENERIC_READ,
            FILE_SHARE_READ,
            OPEN_EXISTING,
            FILE_FLAG_SEQUENTIAL_SCAN))
        {
            BOOST_THROW_EXCEPTION(FilesystemException("Unable to open file") << FileNameInfo(file));
        }

        try
        {
            const auto hash = Hash(HashAlgorithm::Sha256);

            auto buffer = std::array<BYTE, 1024>();
            DWORD bytesRead = 0;
            static_assert(buffer.size() < MAXDWORD);
            auto readResult = ::ReadFile(fileHandle, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead, 0);
            while (readResult)
            {
                if (bytesRead == 0)
                {
                    break;
                }

                hash.HashData(std::span(buffer.data(), bytesRead));
                readResult = ::ReadFile(fileHandle, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead, 0);
            }

            if (!readResult)
            {
                BOOST_THROW_EXCEPTION(FilesystemException("ReadFile failed") << FileNameInfo(file));
            }

            return hash.GetResult();
        }
        catch (HashException& e)
        {
            e << FileNameInfo(file);
            throw;
        }
    }

    void SetFileTime(std::wstring_view file, FILETIME filetime)
    {
        CAtlFile fileHandle;
        if (S_OK != fileHandle.Create(file.data(),
            GENERIC_WRITE,
            FILE_SHARE_READ,
            OPEN_EXISTING))
        {
            BOOST_THROW_EXCEPTION(FilesystemException("Unable to open file") << FileNameInfo(std::wstring(file)));
        }

        if (!::SetFileTime(fileHandle, nullptr, nullptr, &filetime))
        {
            BOOST_THROW_EXCEPTION(FilesystemException("Unable to set last write time") << FileNameInfo(std::wstring(file)));
        }
    }

    std::vector<BYTE> ReadFile(std::wstring_view file)
    {
        CAtlFile fileHandle;
        if (S_OK != fileHandle.Create(file.data(),
            GENERIC_READ,
            FILE_SHARE_READ,
            OPEN_EXISTING))
        {
            BOOST_THROW_EXCEPTION(FileNotFoundException("Unable to open file") << FileNameInfo(std::wstring(file)));
        }

        ULONGLONG size{};
        if (S_OK != fileHandle.GetSize(size))
        {
            BOOST_THROW_EXCEPTION(FilesystemException("Unable to get file size") << FileNameInfo(std::wstring(file)));
        }

        std::vector<BYTE> content(size);
        // TODO: Support file reads that are greater than MAXDWORD.... or express this limitation better in the method signature
        if (size >= MAXDWORD)
        {
            BOOST_THROW_EXCEPTION(FilesystemException(fmt::format("Unable to read file size greater than {}", MAXDWORD)) << FileNameInfo(std::wstring(file)));
        }

        if (S_OK != fileHandle.Read(content.data(), static_cast<DWORD>(size)))
        {
            BOOST_THROW_EXCEPTION(FilesystemException("Unable to read file") << FileNameInfo(std::wstring(file)));
        }

        return content;
    }

    void WriteFile(std::wstring_view file, const void* data, size_t size)
    {
        CAtlFile fileHandle;
        if (S_OK != fileHandle.Create(file.data(),
            GENERIC_WRITE,
            FILE_SHARE_READ,
            CREATE_ALWAYS))
        {
            BOOST_THROW_EXCEPTION(FilesystemException("Unable to open file") << FileNameInfo(std::wstring(file)));
        }

        // TODO: Support file reads that are greater than MAXDWORD.... or express this limitation better in the method signature
        if (size >= MAXDWORD)
        {
            BOOST_THROW_EXCEPTION(FilesystemException(fmt::format("Unable to read file size greater than {}", MAXDWORD)) << FileNameInfo(std::wstring(file)));
        }

        if (S_OK != fileHandle.Write(data, static_cast<DWORD>(size)))
        {
            BOOST_THROW_EXCEPTION(FilesystemException("Unable to write file") << FileNameInfo(std::wstring(file)));
        }
    }
}