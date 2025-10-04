# Unit Tests

# Helper function to create unit tests
function(add_unit_test test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name}
        PRIVATE
            GTest::gtest
            GTest::gtest_main
            AthiVegam::Core
    )
    
    # Add to CTest
    add_test(NAME ${test_name} COMMAND ${test_name})
    
    # Set working directory to binary dir
    set_tests_properties(${test_name} PROPERTIES
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endfunction()

# Core module tests (will be added as we implement)
# add_unit_test(TestFrameArena TestFrameArena.cpp)
# add_unit_test(TestPoolAllocator TestPoolAllocator.cpp)
# add_unit_test(TestSlabAllocator TestSlabAllocator.cpp)
# add_unit_test(TestTime TestTime.cpp)
# add_unit_test(TestFilesystem TestFilesystem.cpp)

# Placeholder test to ensure build system works
add_unit_test(TestPlaceholder TestPlaceholder.cpp)

# Jobs module tests
add_subdirectory(Jobs)

# Comm module tests
add_subdirectory(Comm)

