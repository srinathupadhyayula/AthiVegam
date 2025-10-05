# Core Module Unit Tests

# Reuse helper from parent CMake (add_unit_test)

# Memory allocator tests
add_unit_test(TestFrameArena
    TestFrameArena.cpp
)

target_link_libraries(TestFrameArena
    PRIVATE
        AthiVegam::Core
)

add_unit_test(TestPoolAllocator
    TestPoolAllocator.cpp
)

target_link_libraries(TestPoolAllocator
    PRIVATE
        AthiVegam::Core
)

add_unit_test(TestSlabAllocator
    TestSlabAllocator.cpp
)

target_link_libraries(TestSlabAllocator
    PRIVATE
        AthiVegam::Core
)

# Threading primitives tests
add_unit_test(TestThreading
    TestThreading.cpp
)

target_link_libraries(TestThreading
    PRIVATE
        AthiVegam::Core
)
