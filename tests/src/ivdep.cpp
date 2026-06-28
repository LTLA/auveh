#include <gtest/gtest.h>

#include <random>

#include "auveh/ivdep.hpp"

// These tests won't be meaningful outside of higher optimization levels where auto-vectorization is enabled.
// But might as well just check that the pragmas are set properly.

TEST(Ivdep, Basic) {
    std::mt19937_64 rng;
    std::normal_distribution<> dist;
    std::vector<double> x(100);
    for (auto& x_ : x) {
        x_ = dist(rng);
    }
    std::vector<double> y(100);
    for (auto& y_ : y) {
        y_ = dist(rng);
    }

    auto expected = x;
    for (std::size_t i = 0, end = x.size(); i < end; ++i) {
        expected[i] += y[i] * y[i];
    }

    AUVEH_IVDEP for (std::size_t i = 0, end = x.size(); i < end; ++i) {
        x[i] += y[i] * y[i];
    }

    EXPECT_EQ(expected, x);
}

TEST(Ivdep, Indexed) {
    std::mt19937_64 rng;
    std::normal_distribution<> dist;
    std::vector<double> x(100);
    for (auto& x_ : x) {
        x_ = dist(rng);
    }

    std::vector<int> idx;
    std::vector<double> y;
    std::uniform_real_distribution<> udist;
    for (int i = 0; i < 100; ++i) {
        if (udist(rng)) {
            idx.push_back(i);
            y.push_back(dist(rng));
        }
    }

    auto expected = y;
    for (std::size_t i = 0, end = idx.size(); i < end; ++i) {
        expected[i] += x[idx[i]];
    }

    AUVEH_IVDEP for (std::size_t i = 0, end = idx.size(); i < end; ++i) {
        y[i] += x[idx[i]];
    }

    EXPECT_EQ(expected, y);
}
