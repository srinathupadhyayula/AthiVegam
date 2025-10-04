# Jobs unit tests

# Basic API smoke test
add_unit_test(TestJobsBasic TestJobsBasic.cpp)

target_link_libraries(TestJobsBasic
    PRIVATE
        AthiVegam_Jobs
)
