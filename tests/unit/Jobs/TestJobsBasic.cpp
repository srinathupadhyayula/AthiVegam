#include <gtest/gtest.h>
#include <atomic>
#include "Jobs/Scheduler.hpp"

using namespace Engine::Jobs;

TEST(Jobs_Basic, SubmitAndWait_SynchronousStub)
{
    Scheduler& sched = Scheduler::Instance();
    sched.Initialize();

    std::atomic<int> counter{0};

    JobDesc desc{
        .name = "Increment",
        .priority = JobPriority::Normal
    };

    JobHandle h = sched.Submit(desc, [&]{ counter.fetch_add(1, std::memory_order_relaxed); });
    sched.Wait(h);

    EXPECT_EQ(counter.load(), 1);
    EXPECT_TRUE(sched.IsInitialized());

    sched.Shutdown();
}

TEST(Jobs_Basic, ParallelFor_SynchronousBaseline)
{
    Scheduler& sched = Scheduler::Instance();
    sched.Initialize();

    constexpr std::size_t N = 10;
    std::array<int, N> arr{};

    sched.ParallelFor(0, N, 1, [&](std::size_t i){ arr[i] = static_cast<int>(i * 2); });

    for (std::size_t i = 0; i < N; ++i) {
        EXPECT_EQ(arr[i], static_cast<int>(i * 2));
    }

    sched.Shutdown();
}
