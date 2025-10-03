# EngineTest - Comprehensive Phase 0 Foundation Test

add_executable(EngineTest
    main.cpp
)

target_link_libraries(EngineTest
    PRIVATE
        AthiVegam::Core
        AthiVegam_Comm
)

# Set output directory
set_target_properties(EngineTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Copy to Debug/Release subdirectories for Visual Studio
if(MSVC)
    set_target_properties(EngineTest PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin/RelWithDebInfo"
    )
endif()

