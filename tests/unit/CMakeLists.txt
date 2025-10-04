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

# Core module tests
add_subdirectory(Core)

# Placeholder test to ensure build system works
add_unit_test(TestPlaceholder TestPlaceholder.cpp)

# Jobs module tests
add_subdirectory(Jobs)

# Comm module tests
add_subdirectory(Comm)

# ECS module tests
add_subdirectory(ECS)
