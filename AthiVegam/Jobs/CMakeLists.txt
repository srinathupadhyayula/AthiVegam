# AthiVegam Jobs Module
# Work-stealing job scheduler with hazard tracking

# Source files
set(JOBS_SOURCES
    Scheduler.cpp
    HazardTracker.cpp
    Fiber.cpp
)

# Header files
set(JOBS_HEADERS
    Types.hpp
    Scheduler.hpp
    HazardTracker.hpp
    Fiber.hpp
)

# Create library
add_library(AthiVegam_Jobs STATIC
    ${JOBS_SOURCES}
    ${JOBS_HEADERS}
)

# Include directories
target_include_directories(AthiVegam_Jobs
    PUBLIC
        ${CMAKE_SOURCE_DIR}
)

# Link dependencies
target_link_libraries(AthiVegam_Jobs
    PUBLIC
        AthiVegam_Core
    PRIVATE
        spdlog::spdlog
)

# Compiler settings
if(MSVC)
    target_compile_options(AthiVegam_Jobs PRIVATE
        /W4
        /permissive-
    )
else()
    target_compile_options(AthiVegam_Jobs PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# Set target properties
set_target_properties(AthiVegam_Jobs PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    FOLDER "Engine"
)

# Create alias for consistent naming
add_library(AthiVegam::Jobs ALIAS AthiVegam_Jobs)

