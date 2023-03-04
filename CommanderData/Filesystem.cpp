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

                const auto dwError = GetLastError();
                if (dwError != ERROR_NO_MORE_FILES)
                {
                    BOOST_THROW_EXCEPTION(FilesystemException("Unable to finish iterating directory") << FilePathInfo(dir));
                }

                FindClose(findHandle);
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

                const auto lastError = GetLastError();
                if (lastError != ERROR_HANDLE_EOF)
                {
                    BOOST_THROW_EXCEPTION(FilesystemException("Unable to finish iterating file streams") << FileNameInfo(file));
                }

                FindClose(findHandle);
            }
        );
    }

    std::vector<BYTE> HashFileContents(std::wstring file)
    {
        HANDLE fileHandle = CreateFileW(
            file.data(),
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_SEQUENTIAL_SCAN,
            NULL);

        if (fileHandle == INVALID_HANDLE_VALUE)
        {
            BOOST_THROW_EXCEPTION(FilesystemException("Unable to open file") << FileNameInfo(file));
        }

        HCRYPTPROV cryptoProvider = NULL;
        if (!CryptAcquireContext(&cryptoProvider,
            NULL,
            NULL,
            PROV_RSA_AES,
            CRYPT_VERIFYCONTEXT))
        {
            CloseHandle(fileHandle);
            BOOST_THROW_EXCEPTION(FilesystemException("CryptAcquireContext failed") << FileNameInfo(file));
        }

        HCRYPTPROV hashHandle = NULL;
        if (!CryptCreateHash(cryptoProvider, CALG_SHA_256, 0, 0, &hashHandle))
        {
            CloseHandle(fileHandle);
            CryptReleaseContext(cryptoProvider, 0);
            BOOST_THROW_EXCEPTION(FilesystemException("CryptCreateHash failed") << FileNameInfo(file));
        }

        auto buffer = std::array<BYTE, 1024>();
        DWORD bytesRead = 0;
        auto readResult = ReadFile(fileHandle, buffer.data(), buffer.size(), &bytesRead, 0);
        while (readResult)
        {
            if (bytesRead == 0)
            {
                break;
            }

            if (!CryptHashData(hashHandle, buffer.data(), bytesRead, 0))
            {
                CryptReleaseContext(cryptoProvider, 0);
                CryptDestroyHash(hashHandle);
                CloseHandle(fileHandle);
                BOOST_THROW_EXCEPTION(FilesystemException("CtyptHashData failed") << FileNameInfo(file));
            }

            readResult = ReadFile(fileHandle, buffer.data(), buffer.size(), &bytesRead, 0);
        }

        if (!readResult)
        {
            CryptReleaseContext(cryptoProvider, 0);
            CryptDestroyHash(hashHandle);
            CloseHandle(fileHandle);
            BOOST_THROW_EXCEPTION(FilesystemException("ReadFile failed") << FileNameInfo(file));
        }

        DWORD hashSize = 0;
        DWORD count = sizeof(DWORD);
        if (!CryptGetHashParam(hashHandle, HP_HASHSIZE, reinterpret_cast<BYTE*>(&hashSize), &count, 0))
            BOOST_THROW_EXCEPTION(FilesystemException("CryptGetHashParam HP_HASHSIZE failed") << FileNameInfo(file));

        std::vector<BYTE> hashOutputBuffer(hashSize);
        if (!CryptGetHashParam(hashHandle, HP_HASHVAL, hashOutputBuffer.data(), &hashSize, 0))
            BOOST_THROW_EXCEPTION(FilesystemException("CryptGetHashParam HP_HASHVAL failed") << FileNameInfo(file));

        return hashOutputBuffer;
    }

    std::wstring GetSHA256(const WCHAR* file)
    {
        HANDLE fileHandle = CreateFileW(
            file,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_SEQUENTIAL_SCAN,
            NULL);

        if (fileHandle == INVALID_HANDLE_VALUE)
        {
            BOOST_THROW_EXCEPTION(FilesystemException("Unable to open file") << FileNameInfo(file));
        }

        HCRYPTPROV cryptoProvider = NULL;
        if (!CryptAcquireContext(&cryptoProvider,
            NULL,
            NULL,
            PROV_RSA_AES,
            CRYPT_VERIFYCONTEXT))
        {
            CloseHandle(fileHandle);
            BOOST_THROW_EXCEPTION(FilesystemException("CryptAcquireContext failed") << FileNameInfo(file));
        }

        HCRYPTPROV hashHandle = NULL;
        if (!CryptCreateHash(cryptoProvider, CALG_SHA_256, 0, 0, &hashHandle))
        {
            CloseHandle(fileHandle);
            CryptReleaseContext(cryptoProvider, 0);
            BOOST_THROW_EXCEPTION(FilesystemException("CryptCreateHash failed") << FileNameInfo(file));
        }
        
        auto buffer = std::array<BYTE, 1024>();
        DWORD bytesRead = 0;
        auto readResult = ReadFile(fileHandle, buffer.data(), buffer.size(), &bytesRead, 0);
        while (readResult)
        {
            if (bytesRead == 0)
            {
                break;
            }

            if (!CryptHashData(hashHandle, buffer.data(), bytesRead, 0))
            {
                CryptReleaseContext(cryptoProvider, 0);
                CryptDestroyHash(hashHandle);
                CloseHandle(fileHandle);
                BOOST_THROW_EXCEPTION(FilesystemException("CtyptHashData failed") << FileNameInfo(file));
            }

            readResult = ReadFile(fileHandle, buffer.data(), buffer.size(), &bytesRead, 0);
        }

        if (!readResult)
        {
            CryptReleaseContext(cryptoProvider, 0);
            CryptDestroyHash(hashHandle);
            CloseHandle(fileHandle);
            BOOST_THROW_EXCEPTION(FilesystemException("ReadFile failed") << FileNameInfo(file));
        }

        DWORD cbHashSize = 0;
        DWORD dwCount = sizeof(DWORD);
        if (!CryptGetHashParam(hashHandle, HP_HASHSIZE, reinterpret_cast<BYTE*>(&cbHashSize), &dwCount, 0))
            return {};

        // NOTE: We don't expect to need to hash anything bigger than SHA256
        static constexpr auto MAX_HASH_OUTPUT_SIZE = 32;
        auto hashOutputBuffer = std::array<BYTE, MAX_HASH_OUTPUT_SIZE>();
        if (!CryptGetHashParam(hashHandle, HP_HASHVAL, hashOutputBuffer.data(), &cbHashSize, 0))
            return {};

        std::wostringstream oss;
        oss.fill('0');
        oss << std::hex;
        for (const auto& b : hashOutputBuffer)
        {
            oss << std::setw(2) << static_cast<const unsigned int>(b);
        }

        CryptReleaseContext(cryptoProvider, 0);
        CryptDestroyHash(hashHandle);
        CloseHandle(fileHandle);
        return oss.str();
    }

    void WriteFile(std::wstring_view file, const void* data, size_t size)
    {
        CAtlFile fileHandle;
        if (S_OK != fileHandle.Create(file.data(),
            GENERIC_WRITE,
            FILE_SHARE_READ,
            CREATE_ALWAYS))
        {
            auto err = GetLastError();
            BOOST_THROW_EXCEPTION(FilesystemException("Unable to open file") << FileNameInfo2(file));
        }

        if (S_OK != fileHandle.Write(data, size))
        {
            BOOST_THROW_EXCEPTION(FilesystemException("Unable to write file") << FileNameInfo2(file));
        }

        fileHandle.Close();
    }
}