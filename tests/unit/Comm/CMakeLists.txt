# Comm Layer Unit Tests

add_executable(TestCommLayer
    TestCommLayer.cpp
)

target_link_libraries(TestCommLayer
    PRIVATE
        AthiVegam_Comm
        AthiVegam_Core
        GTest::gtest
        GTest::gtest_main
)

# Add to CTest
add_test(NAME TestCommLayer COMMAND TestCommLayer)

# Set working directory for tests
set_target_properties(TestCommLayer PROPERTIES
    FOLDER "Tests/Unit"
)

