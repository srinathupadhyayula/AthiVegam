# ECS Unit Tests

# Reuse helper from parent CMake (add_unit_test)

add_unit_test(TestECS_EntityLifecycle
    TestECS_EntityLifecycle.cpp
)

target_link_libraries(TestECS_EntityLifecycle
    PRIVATE
        AthiVegam::ECS
)

add_unit_test(TestECS_ComponentSystem
    TestECS_ComponentSystem.cpp
)

target_link_libraries(TestECS_ComponentSystem
    PRIVATE
        AthiVegam::ECS
)

add_unit_test(TestECS_ComponentOperations
    TestECS_ComponentOperations.cpp
)

target_link_libraries(TestECS_ComponentOperations
    PRIVATE
        AthiVegam::ECS
)

add_unit_test(TestECS_Query
    TestECS_Query.cpp
)

target_link_libraries(TestECS_Query
    PRIVATE
        AthiVegam::ECS
)

add_unit_test(TestECS_ParallelIteration
    TestECS_ParallelIteration.cpp
)

target_link_libraries(TestECS_ParallelIteration
    PRIVATE
        AthiVegam::ECS
        AthiVegam::Jobs
)
