cmake_minimum_required(VERSION 3.28)

# Project definition
project(AthiVegam
    VERSION 0.1.0
    DESCRIPTION "Modular data-oriented game engine"
    LANGUAGES CXX
)

# C++ Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Global options
option(ATHIVEGAM_BUILD_TESTS "Build unit tests" ON)
option(ATHIVEGAM_BUILD_EXAMPLES "Build examples" ON)
option(ATHIVEGAM_BUILD_DOCS "Build documentation" OFF)
option(ATHIVEGAM_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ATHIVEGAM_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# CMake modules path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Compiler-specific settings
if(MSVC)
    # Unicode support for Windows APIs
    add_compile_definitions(
        UNICODE             # Use Unicode (wide char) Windows APIs
        _UNICODE            # Use Unicode C runtime functions
    )

    # MSVC flags
    add_compile_options(
        /W4                 # Warning level 4
        /permissive-        # Strict conformance
        /std:c++latest      # Latest C++ standard
        /Zc:__cplusplus     # Correct __cplusplus macro
        /Zc:preprocessor    # Conforming preprocessor
        /MP                 # Multi-processor compilation
    )

    # Disable specific warnings
    add_compile_options(
        /wd4100  # Unreferenced formal parameter (common in interfaces)
        /wd4201  # Nonstandard extension used: nameless struct/union
    )
    
    # Debug/Release specific
    add_compile_options($<$<CONFIG:Debug>:/Od>)
    add_compile_options($<$<CONFIG:Debug>:/Zi>)
    add_compile_options($<$<CONFIG:Debug>:/RTC1>)
    add_compile_options($<$<CONFIG:Release>:/O2>)
    add_compile_options($<$<CONFIG:Release>:/Ob2>)
    add_compile_definitions($<$<CONFIG:Release>:NDEBUG>)
    add_compile_options($<$<CONFIG:RelWithDebInfo>:/O2>)
    add_compile_options($<$<CONFIG:RelWithDebInfo>:/Zi>)
    add_compile_definitions($<$<CONFIG:RelWithDebInfo>:NDEBUG>)
    
    # Treat warnings as errors in CI
    if(DEFINED ENV{CI})
        add_compile_options(/WX)
    endif()
else()
    # GCC/Clang flags
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
    )
    
    # Debug/Release specific
    add_compile_options($<$<CONFIG:Debug>:-O0>)
    add_compile_options($<$<CONFIG:Debug>:-g3>)
    add_compile_options($<$<CONFIG:Release>:-O3>)
    add_compile_definitions($<$<CONFIG:Release>:NDEBUG>)
    add_compile_options($<$<CONFIG:RelWithDebInfo>:-O2>)
    add_compile_options($<$<CONFIG:RelWithDebInfo>:-g>)
    add_compile_definitions($<$<CONFIG:RelWithDebInfo>:NDEBUG>)
    
    # Treat warnings as errors in CI
    if(DEFINED ENV{CI})
        add_compile_options(-Werror)
    endif()
endif()

# Sanitizers
if(ATHIVEGAM_ENABLE_ASAN)
    if(MSVC)
        add_compile_options(/fsanitize=address)
        add_link_options(/fsanitize=address)
    else()
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
endif()

if(ATHIVEGAM_ENABLE_UBSAN AND NOT MSVC)
    add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=undefined)
endif()

# Dependencies
include(FetchContent)

# GoogleTest
if(ATHIVEGAM_BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
        GIT_SHALLOW TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
    GIT_SHALLOW TRUE
)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(spdlog)

# mimalloc
FetchContent_Declare(
    mimalloc
    GIT_REPOSITORY https://github.com/microsoft/mimalloc.git
    GIT_TAG v2.1.2
    GIT_SHALLOW TRUE
)
set(MI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(MI_BUILD_OBJECT OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(mimalloc)

# nlohmann_json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(nlohmann_json)

# Engine modules
add_subdirectory(AthiVegam)

# Tests
if(ATHIVEGAM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples
if(ATHIVEGAM_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Documentation
if(ATHIVEGAM_BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs)
        set(DOXYGEN_GENERATE_HTML YES)
        set(DOXYGEN_GENERATE_MAN NO)
        doxygen_add_docs(docs
            ${CMAKE_CURRENT_SOURCE_DIR}/AthiVegam
            COMMENT "Generating API documentation with Doxygen"
        )
    else()
        message(WARNING "Doxygen not found, documentation will not be built")
    endif()
endif()

# Installation rules (future)
# install(TARGETS AthiVegam_Core DESTINATION lib)
# install(DIRECTORY AthiVegam/Core/ DESTINATION include/AthiVegam/Core FILES_MATCHING PATTERN "*.hpp")

# Print configuration summary
message(STATUS "")
message(STATUS "AthiVegam Configuration:")
message(STATUS "  Version:              ${PROJECT_VERSION}")
message(STATUS "  C++ Standard:         C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type:           ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler:             ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Build Tests:          ${ATHIVEGAM_BUILD_TESTS}")
message(STATUS "  Build Examples:       ${ATHIVEGAM_BUILD_EXAMPLES}")
message(STATUS "  Build Docs:           ${ATHIVEGAM_BUILD_DOCS}")
message(STATUS "  AddressSanitizer:     ${ATHIVEGAM_ENABLE_ASAN}")
message(STATUS "  UBSanitizer:          ${ATHIVEGAM_ENABLE_UBSAN}")
message(STATUS "")

