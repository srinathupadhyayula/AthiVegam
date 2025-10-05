# Performance Tests

# Helper function to create performance tests
function(add_performance_test test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name}
        PRIVATE
            GTest::gtest
            GTest::gtest_main
            AthiVegam::Core
            AthiVegam::Jobs
    )
    
    # Add to CTest
    add_test(NAME ${test_name} COMMAND ${test_name})
    
    # Set working directory to binary dir
    set_tests_properties(${test_name} PROPERTIES
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    
    # Performance tests may take longer
    set_tests_properties(${test_name} PROPERTIES
        TIMEOUT 600
    )
endfunction()

# ECS performance tests
add_subdirectory(ECS)

