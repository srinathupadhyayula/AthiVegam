# Unicode and Wide Character Support in AthiVegam

## Overview

AthiVegam is configured to use Unicode (wide character) APIs on Windows for proper international character support and modern Windows development practices.

## Configuration

### CMake Settings

The project automatically defines the following for MSVC builds:

```cmake
add_compile_definitions(
    UNICODE             # Use Unicode (wide char) Windows APIs
    _UNICODE            # Use Unicode C runtime functions
)
```

This ensures that:
- Windows API calls resolve to their wide character versions (e.g., `MessageBoxW` instead of `MessageBoxA`)
- C runtime functions use wide character versions (e.g., `_wfopen` instead of `fopen`)
- Proper handling of international characters and emoji

## Usage Guidelines

### 1. String Literals

Use `L""` prefix for wide string literals:

```cpp
const wchar_t* wideStr = L"Hello, 世界! 🎮";
```

### 2. String Types

Prefer `std::wstring` over `std::string` when interfacing with Windows APIs:

```cpp
#include <string>

std::wstring gamePath = L"C:\\Games\\AthiVegam\\";
std::wstring playerName = L"Player_日本語";
```

### 3. Windows API Calls

Windows APIs automatically resolve to wide versions:

```cpp
#include <windows.h>

// Automatically calls MessageBoxW due to UNICODE definition
MessageBox(NULL, L"Game started!", L"AthiVegam", MB_OK);

// Automatically calls CreateFileW
HANDLE file = CreateFile(
    L"save.dat",
    GENERIC_READ,
    0,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL
);
```

### 4. Character Conversions

When you need to convert between narrow (UTF-8) and wide (UTF-16) strings:

#### UTF-8 to Wide (UTF-16)

```cpp
#include <windows.h>
#include <string>

std::wstring Utf8ToWide(const std::string& utf8)
{
    if (utf8.empty())
        return std::wstring();
        
    int size = MultiByteToWideChar(
        CP_UTF8,
        0,
        utf8.c_str(),
        static_cast<int>(utf8.size()),
        nullptr,
        0
    );
    
    std::wstring result(size, 0);
    MultiByteToWideChar(
        CP_UTF8,
        0,
        utf8.c_str(),
        static_cast<int>(utf8.size()),
        &result[0],
        size
    );
    
    return result;
}
```

#### Wide (UTF-16) to UTF-8

```cpp
std::string WideToUtf8(const std::wstring& wide)
{
    if (wide.empty())
        return std::string();
        
    int size = WideCharToMultiByte(
        CP_UTF8,
        0,
        wide.c_str(),
        static_cast<int>(wide.size()),
        nullptr,
        0,
        nullptr,
        nullptr
    );
    
    std::string result(size, 0);
    WideCharToMultiByte(
        CP_UTF8,
        0,
        wide.c_str(),
        static_cast<int>(wide.size()),
        &result[0],
        size,
        nullptr,
        nullptr
    );
    
    return result;
}
```

### 5. File I/O

For cross-platform file I/O, use `std::filesystem` which handles Unicode properly:

```cpp
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

// Filesystem paths handle Unicode automatically
fs::path configPath = L"C:\\Users\\ユーザー\\config.json";

// std::fstream works with fs::path on Windows
std::ifstream file(configPath);
```

### 6. Console Output

For console output with Unicode support:

```cpp
#include <iostream>
#include <io.h>
#include <fcntl.h>

// Enable Unicode console output (call once at startup)
void EnableUnicodeConsole()
{
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);
}

// Use std::wcout for wide output
std::wcout << L"Game: AthiVegam 🎮" << std::endl;
```

## Best Practices

### ✅ DO

- Use `wchar_t` and `std::wstring` for Windows-specific code
- Use `L""` prefix for wide string literals
- Use `std::filesystem::path` for file paths (handles Unicode automatically)
- Convert to/from UTF-8 at API boundaries (e.g., JSON, network)
- Use wide character Windows APIs consistently

### ❌ DON'T

- Mix `char*` and `wchar_t*` without proper conversion
- Use ANSI Windows APIs (they're deprecated and don't support Unicode properly)
- Assume `sizeof(wchar_t) == 2` (it's platform-dependent, though 2 on Windows)
- Use `std::string` for Windows file paths (use `std::filesystem::path` instead)
- Forget to convert when interfacing with UTF-8 libraries (JSON, HTTP, etc.)

## Cross-Platform Considerations

### Platform-Specific Code

Use conditional compilation for platform-specific string handling:

```cpp
#ifdef _WIN32
    using PlatformString = std::wstring;
    #define PLATFORM_TEXT(x) L##x
#else
    using PlatformString = std::string;
    #define PLATFORM_TEXT(x) x
#endif

// Usage
PlatformString gameName = PLATFORM_TEXT("AthiVegam");
```

### UTF-8 as Internal Format

For cross-platform code, consider using UTF-8 (`std::string`) internally and converting to wide strings only at Windows API boundaries:

```cpp
class ResourceManager {
public:
    // Internal: UTF-8
    void LoadTexture(const std::string& utf8Path);
    
private:
    // Convert to wide for Windows file API
    void LoadTextureImpl(const std::string& utf8Path)
    {
        #ifdef _WIN32
            std::wstring widePath = Utf8ToWide(utf8Path);
            // Use widePath with Windows APIs
        #else
            // Use utf8Path directly on Unix
        #endif
    }
};
```

## Testing Unicode Support

### Test Cases

Ensure your code handles:

1. **ASCII characters**: `"Hello"`
2. **Latin extended**: `"Café, naïve"`
3. **Cyrillic**: `"Привет"`
4. **CJK**: `"你好, こんにちは, 안녕하세요"`
5. **Emoji**: `"🎮🚀⭐"`
6. **Mixed**: `"Player_日本語_123"`

### Example Test

```cpp
TEST(Unicode, FilePathHandling)
{
    std::wstring testPath = L"C:\\Users\\テスト\\save.dat";
    
    // Should handle Unicode path correctly
    HANDLE file = CreateFile(
        testPath.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    EXPECT_NE(file, INVALID_HANDLE_VALUE);
    CloseHandle(file);
}
```

## Common Issues and Solutions

### Issue: Garbled Text in Console

**Solution**: Enable Unicode console mode (see Console Output section above)

### Issue: File Not Found with Unicode Path

**Solution**: Ensure you're using wide character APIs and `L""` string literals

### Issue: Compilation Errors with Windows APIs

**Solution**: Verify `UNICODE` and `_UNICODE` are defined in CMake configuration

### Issue: String Conversion Crashes

**Solution**: Check for null/empty strings before conversion; ensure proper buffer sizing

## References

- [Microsoft: Unicode in the Windows API](https://docs.microsoft.com/en-us/windows/win32/intl/unicode-in-the-windows-api)
- [Microsoft: Using Unicode Normalization](https://docs.microsoft.com/en-us/windows/win32/intl/using-unicode-normalization-to-represent-strings)
- [C++ Reference: std::wstring](https://en.cppreference.com/w/cpp/string/basic_string)
- [C++ Reference: std::filesystem::path](https://en.cppreference.com/w/cpp/filesystem/path)

## Summary

AthiVegam uses Unicode (wide character) APIs on Windows to ensure:
- ✅ Full international character support
- ✅ Proper handling of modern text (emoji, CJK, etc.)
- ✅ Compliance with Windows best practices
- ✅ Future-proof text handling

Always use `wchar_t`/`std::wstring` for Windows-specific code and convert to/from UTF-8 at API boundaries for cross-platform compatibility.

