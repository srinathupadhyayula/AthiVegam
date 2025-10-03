# Comm Module - Communication Layer

# Collect source files
set(COMM_HEADERS
    Bus.hpp
    Channel.hpp
    Payload.hpp
    Types.hpp
)

set(COMM_SOURCES
    Bus.cpp
    Channel.cpp
)

# Create static library
add_library(AthiVegam_Comm STATIC
    ${COMM_HEADERS}
    ${COMM_SOURCES}
)

# Include directories
target_include_directories(AthiVegam_Comm
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Link dependencies
target_link_libraries(AthiVegam_Comm
    PUBLIC
        AthiVegam_Core
        AthiVegam_Jobs
    PRIVATE
        spdlog::spdlog
)

# Compiler settings
if(MSVC)
    target_compile_options(AthiVegam_Comm PRIVATE
        /W4
        /permissive-
    )
else()
    target_compile_options(AthiVegam_Comm PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# Set target properties
set_target_properties(AthiVegam_Comm PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    FOLDER "Engine"
)

