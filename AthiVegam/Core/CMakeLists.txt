# Core Module - Foundation layer

# Collect source files (will be populated as we implement)
set(CORE_HEADERS
    # Application
    Application.hpp
    EntryPoint.hpp

    # Platform
    Platform/Platform.hpp
    Platform/Time.hpp
    Platform/Filesystem.hpp
    Platform/Threading.hpp

    # Memory
    Memory/Allocators.hpp
    Memory/FrameArena.hpp
    Memory/PoolAllocator.hpp
    Memory/SlabAllocator.hpp
    Memory/Handle.hpp

    # Logging
    Logger.hpp
    Logger.inl

    # Utilities
    Types.hpp
    Span.hpp
    Result.hpp
)

set(CORE_SOURCES
    # Application
    Application.cpp

    # Platform
    Platform/Platform.cpp
    Platform/Time.cpp
    Platform/Filesystem.cpp
    Platform/Threading.cpp

    # Memory
    Memory/Allocators.cpp
    Memory/FrameArena.cpp
    Memory/PoolAllocator.cpp

    # Logging
    Logger.cpp
)

# Create Core library
add_library(AthiVegam_Core STATIC
    ${CORE_HEADERS}
    ${CORE_SOURCES}
)

# Alias for consistent naming
add_library(AthiVegam::Core ALIAS AthiVegam_Core)

# Include directories
target_include_directories(AthiVegam_Core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:include>
)

# Link dependencies
target_link_libraries(AthiVegam_Core
    PUBLIC
        spdlog::spdlog
    PRIVATE
        mimalloc-static
)

# Compiler warnings
set_project_warnings(AthiVegam_Core)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(AthiVegam_Core PUBLIC
        ATHIVEGAM_PLATFORM_WINDOWS
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()

# Build type definitions
target_compile_definitions(AthiVegam_Core PUBLIC
    $<$<CONFIG:Debug>:ATHIVEGAM_DEBUG>
    $<$<CONFIG:Release>:ATHIVEGAM_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:ATHIVEGAM_RELWITHDEBINFO>
)

