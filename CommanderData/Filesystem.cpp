module;

#include <string>
#include <boost/coroutine2/coroutine.hpp>
#include <fmt/core.h>
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

    VOID CALLBACK FileIOCompletionRoutine(
        __in  DWORD dwErrorCode,
        __in  DWORD dwNumberOfBytesTransfered,
        __in  LPOVERLAPPED lpOverlapped)
    {
        throw fmt::format("Error code: {}", dwErrorCode);
    }

    std::string ReadFileContents(const std::wstring& file, size_t size)
    {
        HANDLE fileHandle = CreateFile(
            file.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
            NULL);

        if (fileHandle == INVALID_HANDLE_VALUE)
        {
            throw std::exception("Unable to open file");
        }

        std::string fileContents;
        OVERLAPPED ol = { 0 };
        fileContents.reserve(size + 1);
        fileContents.resize(size);
        if (ReadFileEx(fileHandle, fileContents.data(), size, &ol, FileIOCompletionRoutine) == FALSE)
        {
            CloseHandle(fileHandle);
            throw std::exception("Unable to read file");
        }

        CloseHandle(fileHandle);
        return fileContents;
    }
}