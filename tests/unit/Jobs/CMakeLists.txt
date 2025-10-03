# Jobs Module Unit Tests

# Collect test files
set(JOBS_TEST_SOURCES
    TestScheduler.cpp
)

# Create test executable
add_executable(JobsTests
    ${JOBS_TEST_SOURCES}
)

# Link dependencies
target_link_libraries(JobsTests
    PRIVATE
        AthiVegam_Jobs
        AthiVegam_Core
        GTest::gtest
        GTest::gtest_main
)

# Include GoogleTest module for test discovery
include(GoogleTest)

# Add tests to CTest
gtest_discover_tests(JobsTests)

