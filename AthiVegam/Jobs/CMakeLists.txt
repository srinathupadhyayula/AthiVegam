# Jobs Module - Work-stealing Scheduler

set(JOBS_HEADERS
    Types.hpp
    Scheduler.hpp
    Hazards.hpp
    Fiber.hpp
)

set(JOBS_SOURCES
    Scheduler.cpp
    Hazards.cpp
    Fiber.cpp
)

add_library(AthiVegam_Jobs STATIC
    ${JOBS_HEADERS}
    ${JOBS_SOURCES}
)

# Include directories
target_include_directories(AthiVegam_Jobs
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Link dependencies
target_link_libraries(AthiVegam_Jobs
    PUBLIC
        AthiVegam_Core
)

# Warnings helper (if available)
if(COMMAND set_project_warnings)
    set_project_warnings(AthiVegam_Jobs)
endif()

# C++ standard
set_target_properties(AthiVegam_Jobs PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    FOLDER "Engine"
)
