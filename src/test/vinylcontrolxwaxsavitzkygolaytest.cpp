#include <gtest/gtest.h>

#include <algorithm>
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

TEST(VinylControlXwaxSavitzkyGolayTest, LowFrequencySignal) {
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

TEST(VinylControlXwaxSavitzkyGolayTest, PlainSequence) {
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

TEST(VinylControlXwaxSavitzkyGolayTest, NoiseFiltering) {
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

// The window size is chosen so the carrier passes the filter within -3 dB at
// up to 4 times the reference speed (SAVGOL_PASSBAND_PITCH in timecoder.c).
size_t windowSize(double carrierHz, double sampleRate) {
    return savgol_window_size(3, 4.0 * carrierHz / sampleRate);
}

TEST(VinylControlXwaxLibSavitzkyGolayTest, WindowSizeOfEmpiricallyTunedTimecodes) {
    // Verify the empirically tuned windows for Traktor MK2 and Serato stay untouched
    // for different sampling rates
    EXPECT_EQ(5, windowSize(2500, 44100));
    EXPECT_EQ(5, windowSize(2500, 48000));
    EXPECT_EQ(9, windowSize(2500, 96000));
    EXPECT_EQ(11, windowSize(1000, 44100));
}

TEST(VinylControlXwaxLibSavitzkyGolayTest, WindowSizeIsAtLeastTheSmallestValidWindow) {
    // Traktor MK2 CD (3000 Hz) at 44.1 kHz: no window passes 12 kHz, so the smallest window
    // for a 3rd order polynomial is used instead of an invalid one.
    EXPECT_EQ(5, windowSize(3000, 44100));
    EXPECT_EQ(5, savgol_window_size(3, 0.5));
    EXPECT_EQ(5, savgol_window_size(4, 0.5));
    EXPECT_EQ(3, savgol_window_size(1, 0.5));
    EXPECT_EQ(5, savgol_window_size(3, 1.0));
}

TEST(VinylControlXwaxLibSavitzkyGolayTest, WindowSizeGrowsWithSampleRate) {
    size_t previous = 0;

    for (double sampleRate : {44100.0, 48000.0, 88200.0, 96000.0}) {
        const size_t window = windowSize(1000, sampleRate);
        EXPECT_EQ(1u, window % 2) << sampleRate;
        EXPECT_GE(window, previous) << sampleRate;
        previous = window;
    }

    EXPECT_GT(previous, 11u);
}

TEST(VinylControlXwaxLibSavitzkyGolayTest, WindowSizeIsLargestWindowPassingCutoff) {
    // The timecode signal should survive the filter at a pitch of 4.0, so attenuate less than -3dB
    const double cutoff = 4.0 * 1000 / 96000;
    const size_t window = savgol_window_size(3, cutoff);
    const size_t N = 2000; // Number of samples

    auto amplitude = [cutoff](size_t windowSize) {
        struct savitzky_golay* sg = savgol_create(windowSize, 3);
        int peak = 0;

        // Feed a sinusoid with the cutoff frequency into the filter y(n) = A * sin(2π*f*n)

        for (size_t n = 0; n < N; n++) {
            int y = savgol(sg, to_q1_31(0.5 * sin(2 * M_PI * cutoff * n)));

            if (n > N / 2) {
                peak = std::max(peak, std::abs(y));
            }
        }

        savgol_destroy(sg);

        return static_cast<double>(peak) / to_q1_31(0.5);
    };

    EXPECT_GE(amplitude(window), M_SQRT1_2 - 0.01);
    EXPECT_LT(amplitude(window + 2), M_SQRT1_2);
}

} // namespace
