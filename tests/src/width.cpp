#include <gtest/gtest.h>

#include "auveh/width.hpp"

TEST(Width, Elements) {
    EXPECT_GT(auveh::width<int>(), 0);
    EXPECT_GT(auveh::width<double>(), 0);
    EXPECT_GT(auveh::width<char>(), 0);
}
