#include "Core/Platform/Filesystem.hpp"

#include <windows.h>
#include <pathcch.h>
#include <spdlog/spdlog.h>
#include <vector>
#include <algorithm>

#pragma comment(lib, "Pathcch.lib")

namespace Engine::Filesystem {

namespace {
    // Helper to convert UTF-8 to wide string
    std::wstring Utf8ToWide(std::string_view utf8)
    {
        if (utf8.empty())
        {
            return std::wstring();
        }

        int wideSize = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);
        if (wideSize == 0)
        {
            return std::wstring();
        }

        std::wstring wide(wideSize, 0);
        MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), wide.data(), wideSize);
        return wide;
    }

    // Helper to convert wide string to UTF-8
    std::string WideToUtf8(const std::wstring& wide)
    {
        if (wide.empty())
        {
            return std::string();
        }

        int utf8Size = WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), nullptr, 0, nullptr, nullptr);
        if (utf8Size == 0)
        {
            return std::string();
        }

        std::string utf8(utf8Size, 0);
        WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), utf8.data(), utf8Size, nullptr, nullptr);
        return utf8;
    }

    // Helper to get last Windows error as Error
    Error GetLastWindowsError(const std::string& context)
    {
        DWORD errorCode = GetLastError();
        return Error(std::error_code(static_cast<int>(errorCode), std::system_category()), context);
    }
}

Result<FileHandle> OpenFile(std::string_view path, OpenMode mode)
{
    DWORD desiredAccess = 0;
    DWORD creationDisposition = 0;

    // Handle access flags
    if (mode & OpenMode::Read)
    {
        desiredAccess |= GENERIC_READ;
    }
    if (mode & OpenMode::Write)
    {
        desiredAccess |= GENERIC_WRITE;
    }
    if (mode & OpenMode::Append)
    {
        desiredAccess |= FILE_APPEND_DATA;
    }

    // Establish precedence for creation disposition
    if (mode & OpenMode::Append)
    {
        creationDisposition = OPEN_ALWAYS;
    }
    else if (mode & OpenMode::Truncate)
    {
        // Ensure file is created or truncated
        creationDisposition = CREATE_ALWAYS;
    }
    else if (mode & OpenMode::Write)
    {
        creationDisposition = CREATE_ALWAYS;
    }
    else
    {
        creationDisposition = OPEN_EXISTING;
    }

    std::wstring widePath = Utf8ToWide(path);
    HANDLE handle = CreateFileW(
        widePath.c_str(),
        desiredAccess,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        creationDisposition,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (handle == INVALID_HANDLE_VALUE)
    {
        return Err<FileHandle>(GetLastWindowsError("Failed to open file"));
    }

    return Ok(reinterpret_cast<void*>(handle));
}

void CloseFile(FileHandle handle)
{
    if (handle != nullptr)
    {
        CloseHandle(reinterpret_cast<HANDLE>(handle));
    }
}

Result<usize> ReadFile(FileHandle handle, Span<byte> buffer)
{
    if (handle == nullptr)
    {
        return Err<usize>(Error("Invalid file handle"));
    }

    DWORD bytesRead = 0;
    BOOL success = ::ReadFile(
        reinterpret_cast<HANDLE>(handle),
        buffer.data(),
        static_cast<DWORD>(buffer.size()),
        &bytesRead,
        nullptr
    );

    if (!success)
    {
        return Err<usize>(GetLastWindowsError("Failed to read file"));
    }

    return Ok(static_cast<usize>(bytesRead));
}

Result<usize> WriteFile(FileHandle handle, Span<const byte> data)
{
    if (handle == nullptr)
    {
        return Err<usize>(Error("Invalid file handle"));
    }

    DWORD bytesWritten = 0;
    BOOL success = ::WriteFile(
        reinterpret_cast<HANDLE>(handle),
        data.data(),
        static_cast<DWORD>(data.size()),
        &bytesWritten,
        nullptr
    );

    if (!success)
    {
        return Err<usize>(GetLastWindowsError("Failed to write file"));
    }

    return Ok(static_cast<usize>(bytesWritten));
}

Result<u64> GetFileSize(FileHandle handle)
{
    if (handle == nullptr)
    {
        return Err<u64>(Error("Invalid file handle"));
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(reinterpret_cast<HANDLE>(handle), &fileSize))
    {
        return Err<u64>(GetLastWindowsError("Failed to get file size"));
    }

    return Ok(static_cast<u64>(fileSize.QuadPart));
}

Result<u64> SeekFile(FileHandle handle, i64 offset, bool fromEnd)
{
    if (handle == nullptr)
    {
        return Err<u64>(Error("Invalid file handle"));
    }

    LARGE_INTEGER distance;
    distance.QuadPart = offset;

    LARGE_INTEGER newPosition;
    DWORD moveMethod = fromEnd ? FILE_END : FILE_BEGIN;

    if (!SetFilePointerEx(reinterpret_cast<HANDLE>(handle), distance, &newPosition, moveMethod))
    {
        return Err<u64>(GetLastWindowsError("Failed to seek file"));
    }

    return Ok(static_cast<u64>(newPosition.QuadPart));
}

Result<u64> TellFile(FileHandle handle)
{
    if (handle == nullptr)
    {
        return Err<u64>(Error("Invalid file handle"));
    }

    LARGE_INTEGER distance;
    distance.QuadPart = 0;

    LARGE_INTEGER position;
    if (!SetFilePointerEx(reinterpret_cast<HANDLE>(handle), distance, &position, FILE_CURRENT))
    {
        return Err<u64>(GetLastWindowsError("Failed to get file position"));
    }

    return Ok(static_cast<u64>(position.QuadPart));
}

Result<std::vector<byte>> ReadEntireFile(std::string_view path)
{
    auto handleResult = OpenFile(path, OpenMode::Read);
    if (!handleResult)
    {
        return Err<std::vector<byte>>(handleResult.error());
    }

    FileHandle handle = handleResult.value();

    auto sizeResult = GetFileSize(handle);
    if (!sizeResult)
    {
        CloseFile(handle);
        return Err<std::vector<byte>>(sizeResult.error());
    }

    u64 fileSize = sizeResult.value();
    std::vector<byte> buffer(static_cast<usize>(fileSize));

    auto readResult = ReadFile(handle, Span<byte>(buffer.data(), buffer.size()));
    CloseFile(handle);

    if (!readResult)
    {
        return Err<std::vector<byte>>(readResult.error());
    }

    return Ok(std::move(buffer));
}

Result<void> WriteEntireFile(std::string_view path, Span<const byte> data)
{
    auto handleResult = OpenFile(path, OpenMode::Write);
    if (!handleResult)
    {
        return std::unexpected(handleResult.error());
    }

    FileHandle handle = handleResult.value();

    auto writeResult = WriteFile(handle, data);
    CloseFile(handle);

    if (!writeResult)
    {
        return std::unexpected(writeResult.error());
    }

    if (writeResult.value() != data.size())
    {
        return std::unexpected(Error("Failed to write all data"));
    }

    return Ok();
}

bool FileExists(std::string_view path)
{
    std::wstring widePath = Utf8ToWide(path);
    DWORD attributes = GetFileAttributesW(widePath.c_str());
    return (attributes != INVALID_FILE_ATTRIBUTES) && !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

bool DirectoryExists(std::string_view path)
{
    std::wstring widePath = Utf8ToWide(path);
    DWORD attributes = GetFileAttributesW(widePath.c_str());
    return (attributes != INVALID_FILE_ATTRIBUTES) && (attributes & FILE_ATTRIBUTE_DIRECTORY);
}

Result<void> CreateDirectory(std::string_view path)
{
    std::wstring widePath = Utf8ToWide(path);
    if (!CreateDirectoryW(widePath.c_str(), nullptr))
    {
        DWORD error = GetLastError();
        if (error != ERROR_ALREADY_EXISTS)
        {
            return std::unexpected(GetLastWindowsError("Failed to create directory"));
        }
    }
    return Ok();
}

Result<void> RemoveFile(std::string_view path)
{
    std::wstring widePath = Utf8ToWide(path);
    if (!DeleteFileW(widePath.c_str()))
    {
        return std::unexpected(GetLastWindowsError("Failed to delete file"));
    }
    return Ok();
}

Result<void> DeleteDirectory(std::string_view path)
{
    std::wstring widePath = Utf8ToWide(path);
    if (!RemoveDirectoryW(widePath.c_str()))
    {
        return std::unexpected(GetLastWindowsError("Failed to delete directory"));
    }
    return Ok();
}

std::string GetCurrentDirectory()
{
    DWORD bufferSize = GetCurrentDirectoryW(0, nullptr);
    if (bufferSize == 0)
    {
        return std::string();
    }

    std::wstring buffer(bufferSize, 0);
    GetCurrentDirectoryW(bufferSize, buffer.data());
    return WideToUtf8(buffer);
}

Result<void> SetCurrentDirectory(std::string_view path)
{
    std::wstring widePath = Utf8ToWide(path);
    if (!SetCurrentDirectoryW(widePath.c_str()))
    {
        return std::unexpected(GetLastWindowsError("Failed to set current directory"));
    }
    return Ok();
}

std::string GetAbsolutePath(std::string_view path)
{
    std::wstring widePath = Utf8ToWide(path);
    DWORD bufferSize = GetFullPathNameW(widePath.c_str(), 0, nullptr, nullptr);
    if (bufferSize == 0)
    {
        return std::string(path);
    }

    std::wstring buffer(bufferSize, L'\0');
    DWORD written = GetFullPathNameW(widePath.c_str(), bufferSize, buffer.data(), nullptr);
    if (written == 0)
    {
        return std::string(path);
    }
    // Exclude the terminating NUL if present
    std::wstring_view view(buffer.data(), written);
    return WideToUtf8(std::wstring(view));
}

std::string_view GetExtension(std::string_view path)
{
    auto pos = path.find_last_of('.');
    if (pos == std::string_view::npos || pos == path.size() - 1)
    {
        return std::string_view();
    }
    // Return extension without the dot
    return path.substr(pos + 1);
}

std::string_view GetFilename(std::string_view path)
{
    auto pos = path.find_last_of("/\\");
    if (pos == std::string_view::npos)
    {
        return path;
    }
    return path.substr(pos + 1);
}

std::string GetDirectory(std::string_view path)
{
    auto pos = path.find_last_of("/\\");
    if (pos == std::string_view::npos)
    {
        return std::string();
    }
    return std::string(path.substr(0, pos));
}

std::string JoinPath(Span<const std::string_view> components)
{
    if (components.empty())
    {
        return std::string();
    }

    usize totalSize = 0;
    for (const auto& component : components)
    {
        totalSize += component.size() + 1; // +1 for separator
    }

    std::string result;
    result.reserve(totalSize);

    for (usize i = 0; i < components.size(); ++i)
    {
        result += components[i];
        if (i < components.size() - 1)
        {
            result += '\\';
        }
    }

    return result;
}

std::string NormalizePath(std::string_view path)
{
    std::wstring widePath = Utf8ToWide(path);
    // Start with a reasonable size and grow if needed
    std::wstring out(512, L'\0');
    HRESULT hr = PathCchCanonicalizeEx(out.data(), out.size(), widePath.c_str(), PATHCCH_NONE);
    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
    {
        // Try with a larger buffer to accommodate extended-length paths
        out.assign(32768, L'\0');
        hr = PathCchCanonicalizeEx(out.data(), out.size(), widePath.c_str(), PATHCCH_NONE);
    }
    if (FAILED(hr))
    {
        return std::string(path);
    }
    // Ensure proper length without trailing NULs
    size_t len = wcsnlen(out.c_str(), out.size());
    return WideToUtf8(std::wstring(out.c_str(), len));
}

} // namespace Engine::Filesystem

