module;

#include <string>
#include <array>
#include <sstream>
#include <boost/coroutine2/coroutine.hpp>
#include <fmt/core.h>
#include <Windows.h>
#include <iomanip>

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

    boost::coroutines2::coroutine<WIN32_FIND_DATA>::pull_type IterateDirectory(const std::wstring& dir)
    {
        return boost::coroutines2::coroutine<WIN32_FIND_DATA>::pull_type(
            [&](boost::coroutines2::coroutine<WIN32_FIND_DATA>::push_type& sink) {
                WIN32_FIND_DATA findData;
                auto findHandle = FindFirstFileW(dir.c_str(), &findData);
                if (findHandle == INVALID_HANDLE_VALUE)
                {
                    throw std::exception("Unable to iterate directory");
                }
                do
                {
                    sink(findData);
                } while (FindNextFileW(findHandle, &findData) != 0);

                const auto dwError = GetLastError();
                if (dwError != ERROR_NO_MORE_FILES)
                {
                    throw std::exception("Unable to finish iterating directory");
                }
                FindClose(findHandle);
            }
        );
    }

    std::wstring HashFileContents(const WCHAR* file)
    {
        HANDLE fileHandle = CreateFile(
            file,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_SEQUENTIAL_SCAN,
            NULL);

        if (fileHandle == INVALID_HANDLE_VALUE)
        {
            throw std::exception("Unable to open file");
        }

        HCRYPTPROV cryptoProvider = NULL;
        if (!CryptAcquireContext(&cryptoProvider,
            NULL,
            NULL,
            PROV_RSA_AES,
            CRYPT_VERIFYCONTEXT))
        {
            CloseHandle(fileHandle);
            throw std::exception("CryptAcquireContext failed");
        }

        HCRYPTPROV hashHandle = NULL;
        if (!CryptCreateHash(cryptoProvider, CALG_SHA_256, 0, 0, &hashHandle))
        {
            CloseHandle(fileHandle);
            CryptReleaseContext(cryptoProvider, 0);
            throw std::exception("CryptCreateHash failed");
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
                throw std::exception("CtyptHashData failed");
            }

            readResult = ReadFile(fileHandle, buffer.data(), buffer.size(), &bytesRead, 0);
        }

        if (!readResult)
        {
            CryptReleaseContext(cryptoProvider, 0);
            CryptDestroyHash(hashHandle);
            CloseHandle(fileHandle);
            throw std::exception("ReadFile failed");
        }

        DWORD cbHashSize = 0;
        DWORD dwCount = sizeof(DWORD);
        if (!CryptGetHashParam(hashHandle, HP_HASHSIZE, reinterpret_cast<BYTE*>(&cbHashSize), &dwCount, 0))
            return {};

        std::vector<BYTE> hashOutputBuffer(cbHashSize);
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
}