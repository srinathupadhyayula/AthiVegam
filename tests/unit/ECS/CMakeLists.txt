# ECS Unit Tests

# Reuse helper from parent CMake (add_unit_test)

add_unit_test(TestECS_EntityLifecycle
    TestECS_EntityLifecycle.cpp
)

target_link_libraries(TestECS_EntityLifecycle
    PRIVATE
        AthiVegam::ECS
)

