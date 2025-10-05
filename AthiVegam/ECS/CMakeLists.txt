# ECS Module (header-only scaffold for now)
add_library(AthiVegam_ECS INTERFACE)

# Expose include path to consumers; headers live under AthiVegam/
target_include_directories(AthiVegam_ECS
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_library(AthiVegam::ECS ALIAS AthiVegam_ECS)

message(STATUS "Configured AthiVegam::ECS (scaffold)")

