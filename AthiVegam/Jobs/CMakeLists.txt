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
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:include>
)

# Link dependencies
target_link_libraries(AthiVegam_Jobs
    PUBLIC
        AthiVegam::Core
    PRIVATE
        spdlog::spdlog
)

# Compiler warnings
set_project_warnings(AthiVegam_Jobs)

# Set target properties
set_target_properties(AthiVegam_Jobs PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    FOLDER "Engine"
)

# Create alias for consistent naming
add_library(AthiVegam::Jobs ALIAS AthiVegam_Jobs)

