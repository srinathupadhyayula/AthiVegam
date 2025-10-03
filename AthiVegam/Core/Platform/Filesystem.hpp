#pragma once

#include "Core/Types.hpp"
#include "Core/Span.hpp"
#include "Core/Result.hpp"
#include <string>
#include <string_view>
#include <vector>

/// @file Filesystem.hpp
/// @brief Filesystem abstraction for file I/O and path manipulation

namespace Engine::Filesystem {

/// @brief File open modes
enum class OpenMode : u32 {
    Read        = 1 << 0,  ///< Open for reading
    Write       = 1 << 1,  ///< Open for writing
    Append      = 1 << 2,  ///< Append to existing file
    Binary      = 1 << 3,  ///< Binary mode
    Truncate    = 1 << 4,  ///< Truncate existing file
};

/// @brief Bitwise OR for OpenMode
constexpr OpenMode operator|(OpenMode a, OpenMode b) {
    return static_cast<OpenMode>(static_cast<u32>(a) | static_cast<u32>(b));
}

/// @brief Bitwise AND for OpenMode
constexpr bool operator&(OpenMode a, OpenMode b) {
    return (static_cast<u32>(a) & static_cast<u32>(b)) != 0;
}

/// @brief File handle (opaque)
using FileHandle = void*;

/// @brief Invalid file handle constant
inline constexpr FileHandle InvalidFileHandle = nullptr;

/// @brief Open a file
/// @param path File path
/// @param mode Open mode flags
/// @return File handle or error
Result<FileHandle> OpenFile(std::string_view path, OpenMode mode);

/// @brief Close a file
/// @param handle File handle to close
void CloseFile(FileHandle handle);

/// @brief Read from a file
/// @param handle File handle
/// @param buffer Buffer to read into
/// @param size Number of bytes to read
/// @return Number of bytes actually read, or error
Result<usize> ReadFile(FileHandle handle, Span<byte> buffer);

/// @brief Write to a file
/// @param handle File handle
/// @param data Data to write
/// @return Number of bytes written, or error
Result<usize> WriteFile(FileHandle handle, Span<const byte> data);

/// @brief Get file size
/// @param handle File handle
/// @return File size in bytes, or error
Result<u64> GetFileSize(FileHandle handle);

/// @brief Seek to position in file
/// @param handle File handle
/// @param offset Offset in bytes
/// @param fromEnd If true, seek from end of file
/// @return New position, or error
Result<u64> SeekFile(FileHandle handle, i64 offset, bool fromEnd = false);

/// @brief Get current file position
/// @param handle File handle
/// @return Current position in bytes, or error
Result<u64> TellFile(FileHandle handle);

/// @brief Read entire file into memory
/// @param path File path
/// @return File contents as byte vector, or error
Result<std::vector<byte>> ReadEntireFile(std::string_view path);

/// @brief Write entire buffer to file
/// @param path File path
/// @param data Data to write
/// @return Success or error
Result<void> WriteEntireFile(std::string_view path, Span<const byte> data);

/// @brief Check if file exists
/// @param path File path
/// @return true if file exists
bool FileExists(std::string_view path);

/// @brief Check if directory exists
/// @param path Directory path
/// @return true if directory exists
bool DirectoryExists(std::string_view path);

/// @brief Create directory (and parent directories if needed)
/// @param path Directory path
/// @return Success or error
Result<void> CreateDirectory(std::string_view path);

/// @brief Remove/delete file
/// @param path File path
/// @return Success or error
Result<void> RemoveFile(std::string_view path);

/// @brief Delete directory (must be empty)
/// @param path Directory path
/// @return Success or error
Result<void> DeleteDirectory(std::string_view path);

/// @brief Get current working directory
/// @return Current working directory path
std::string GetCurrentDirectory();

/// @brief Set current working directory
/// @param path New working directory
/// @return Success or error
Result<void> SetCurrentDirectory(std::string_view path);

/// @brief Get absolute path from relative path
/// @param path Relative path
/// @return Absolute path
std::string GetAbsolutePath(std::string_view path);

/// @brief Get file extension (including dot)
/// @param path File path
/// @return Extension string (e.g., ".txt")
std::string_view GetExtension(std::string_view path);

/// @brief Get filename without directory
/// @param path File path
/// @return Filename
std::string_view GetFilename(std::string_view path);

/// @brief Get directory from path
/// @param path File path
/// @return Directory path
std::string GetDirectory(std::string_view path);

/// @brief Join path components
/// @param components Path components to join
/// @return Joined path
std::string JoinPath(Span<const std::string_view> components);

/// @brief Normalize path (resolve . and .., convert slashes)
/// @param path Path to normalize
/// @return Normalized path
std::string NormalizePath(std::string_view path);

} // namespace Engine::Filesystem

