#include <gtest/gtest.h>

// Placeholder test to verify build system works
TEST(Placeholder, BasicAssertion) {
    EXPECT_EQ(1 + 1, 2);
    EXPECT_TRUE(true);
}

TEST(Placeholder, StringComparison) {
    std::string hello = "Hello";
    std::string world = "World";
    EXPECT_NE(hello, world);
    EXPECT_EQ(hello + " " + world, "Hello World");
}

