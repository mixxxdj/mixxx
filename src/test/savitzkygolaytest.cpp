#include <gtest/gtest.h>

#include <cmath>
#include <cstdlib>

#ifdef _MSC_VER
#include "filters.h"
#else
extern "C" {
#include "filters.h"
}
#endif

namespace {

#define Q1_31_SCALE (1LL << 31)
typedef int32_t q31_t; // 32-bit fractional data type in 1.31 format

static q31_t to_q1_31(double x) {
    return (q31_t)round(x * Q1_31_SCALE);
}

class SavitzkyGolayTest : public ::testing::Test {
  protected:
    static void SetUpTestSuite() {
    }

    static void TearDownTestSuite() {
    }
};

TEST(SavitzkyGolayTest, LowFrequencySignal) {
    const double f = 100.0;
    const double w = 2 * M_PI * f;
    const double fs = 44100;
    const double Ts = 1 / fs;
    const int N = 100;
    const double atol = 0.2;

    q31_t input[N];
    q31_t output[N];

    for (int n = 0; n < N; n++) {
        input[n] = to_q1_31(sin(w * n * Ts));
    }

    struct savitzky_golay* sg = savgol_create(11, 2);

    for (int n = 0; n < N; n++) {
        output[n] = savgol(sg, input[n]);
    }

    for (int n = 10; n < N; n++) { // skip startup
        EXPECT_NEAR(output[n], input[n], to_q1_31(atol));
    }

    savgol_destroy(sg);
}

TEST(SavitzkyGolayTest, PlainSequence) {
    const int N = 21;
    const double atol = 1e-6;

    q31_t input[N];
    q31_t output[N];

    for (int n = 0; n < N; n++) {
        input[n] = to_q1_31(0.99);
    }

    struct savitzky_golay* sg = savgol_create(11, 2);

    for (int n = 0; n < N; n++) {
        output[n] = savgol(sg, input[n]);
    }

    for (int n = N / 2; n < N; n++) { // skip startup
        EXPECT_NEAR(output[n], input[n], to_q1_31(atol));
    }

    savgol_destroy(sg);
}

TEST(SavitzkyGolayTest, NoiseFiltering) {
    const double f = 100.0;
    const double w = 2 * M_PI * f;
    const double fs = 44100;
    const double Ts = 1 / fs;
    const int N = 100;
    const double atol = 0.2;

    q31_t noise[N];
    q31_t input[N];
    q31_t noisy_input[N];
    q31_t output[N];

    for (int n = 0; n < N; n++) {
        input[n] = to_q1_31(sin(w * n * Ts));
        noise[n] = rand() % (10000000);
        noisy_input[n] = input[n] + noise[n];
    }

    struct savitzky_golay* sg = savgol_create(11, 2);

    for (int n = 0; n < N; n++) {
        output[n] = savgol(sg, noisy_input[n]);
    }

    for (int n = 10; n < N; n++) { // skip startup
        EXPECT_NEAR(output[n], input[n], to_q1_31(atol));
    }

    savgol_destroy(sg);
}

} // namespace
