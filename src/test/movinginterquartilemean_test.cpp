#include <gtest/gtest.h>
#include <QDebug>

#include "util/movinginterquartilemean.h"

namespace {

// This is the current lowest error that lets the class pass
// tests doubles7 and doubles9.
const double kMaxAcceptedError = 0.5e-6;

class MovingInterquartileMeanTest : public ::testing::Test {};

TEST_F(MovingInterquartileMeanTest, zeros1) {
    MovingInterquartileMean iqm(1);
    double mean;
    for (int i = 1; i <= 50; ++i) {
        mean = iqm.insert(0);
        EXPECT_DOUBLE_EQ(0, mean) << "Iteration i=" << i;
    }
    iqm.insert(-4);
    mean = iqm.mean();
    EXPECT_DOUBLE_EQ(-4, mean);
}

TEST_F(MovingInterquartileMeanTest, zeros2) {
    MovingInterquartileMean iqm(2);
    double mean = iqm.insert(0);
    EXPECT_DOUBLE_EQ(0, mean);
    for (int i = 1; i <= 50; ++i) {
        mean = iqm.insert(0);
        EXPECT_DOUBLE_EQ(0, mean) << "Iteration i=" << i;
    }
    iqm.insert(-4);
    mean = iqm.mean();
    EXPECT_DOUBLE_EQ(-2, mean);
}

TEST_F(MovingInterquartileMeanTest, integers1) {
    MovingInterquartileMean iqm(1);
    double mean;
    for (int i = 1; i <= 50; ++i) {
        mean = iqm.insert(i);
        EXPECT_DOUBLE_EQ(i, mean) << "Iteration i=" << i;
    }
    iqm.insert(-4);
    mean = iqm.mean();
    EXPECT_DOUBLE_EQ(-4, mean);
}

TEST_F(MovingInterquartileMeanTest, integers2) {
    MovingInterquartileMean iqm(2);
    double mean = iqm.insert(0);
    EXPECT_DOUBLE_EQ(0, mean);
    for (int i = 1; i <= 50; ++i) {
        mean = iqm.insert(i);
        EXPECT_DOUBLE_EQ((2*i-1)/2.0, mean) << "Iteration i=" <<i;
    }
    iqm.insert(-4);
    mean = iqm.mean();
    EXPECT_DOUBLE_EQ(23, mean);
}

TEST_F(MovingInterquartileMeanTest, integers9) {
    MovingInterquartileMean iqm(9);
    for (int i = 0; i <= 50; ++i) {
        double mean = iqm.insert(i);
        double expected = 0;
        int j;
        for (j = i; j >= 0 && j >= i - 8; --j) {
            expected += j;
        }
        expected /= i - j;
        EXPECT_DOUBLE_EQ(expected, mean) << "Iteration i=" << i;
    }
}

TEST_F(MovingInterquartileMeanTest, doubles7) {
    // TODO(Ferran Pujol): Correctly rewrite all means[i]
    // with the same number of significant figures.
    MovingInterquartileMean iqm(7);
    double input[9] = { 16.345, 2.129674, 77, -40.23, 12071, -12071,
                        -15, 0, 1};
    double means[9] = { 16.345, 9.237337, 31.8248913333, 13.8111685,
                        30.2769022, 12.286558, 3.6191925714,
                        -1.0508074285, -1.3735714285};
    for (int i = 0; i < 9; ++i) {
        double mean = iqm.insert(input[i]);
        //TODO(Ferran Pujol): Why does EXPECT_DOUBLE_EQ fail here?
        EXPECT_NEAR(means[i], mean, kMaxAcceptedError) << "Iteration i=" << i;
    }
}

TEST_F(MovingInterquartileMeanTest, doubles9) {
    MovingInterquartileMean iqm(9);
    double input[15] = { 0.0000567345, 1547.12, 2.12655, 2.12687,
                         -1.12354, -0.988888, 0.759845, 0.325784,
                         -0.00345781, 2.54123, 0.0000001, -0.845548,
                         1.00574, 0.234511, 0.00444412};
    double means[15] = { 0.0000567345, 773.560028367, 516.415535578,
                         387.843369184, 1.48869802035, 0.898532444833,
                         0.905984867, 0.803058933625, 0.595112194889,
                         1.06771958722, 0.595099609444, 0.0573442311111,
                         0.0573442311111, 0.250574553889, 0.252138432222};
    for (int i = 0; i < 15; ++i) {
        double mean = iqm.insert(input[i]);
        //TODO(Ferran Pujol): Why does EXPECT_DOUBLE_EQ fail here?
        EXPECT_NEAR(means[i], mean, kMaxAcceptedError) << "Iteration i=" << i;
    }
}

}  // namespace
