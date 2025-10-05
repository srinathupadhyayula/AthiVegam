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

# Alias for consistent naming
add_library(AthiVegam::Comm ALIAS AthiVegam_Comm)

# Include directories
target_include_directories(AthiVegam_Comm
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:include>
)

# Link dependencies
target_link_libraries(AthiVegam_Comm
    PUBLIC
        AthiVegam::Core
        AthiVegam::Jobs
    PRIVATE
        spdlog::spdlog
)

# Compiler warnings
set_project_warnings(AthiVegam_Comm)

# Set target properties
set_target_properties(AthiVegam_Comm PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    FOLDER "Engine"
)

