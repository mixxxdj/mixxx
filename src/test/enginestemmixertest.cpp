// EngineStemMixer test (S3): bit-exactitud vs escalar (tol 1e-6),
// null-test de fase, rampa click-free y bench AVX2 >= 2x escalar.
//
// Buffers alignas(32) pre-alocados fuera del path medido; process()
// no debe reservar nada (verificado por inspeccion: solo stack+SIMD).

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <new>
#include <numeric>
#include <random>
#include <vector>

#include "engine/stems/enginestemmixer.h"

namespace {

using GainArray = EngineStemMixer::GainArray;
constexpr float kTol = 1e-6f;
constexpr SINT kFrames = 1024;

// Buffer 32B-aligned pre-alocado (nunca dentro del hot loop).
struct AlignedBuffer {
    CSAMPLE* data = nullptr;
    SINT size = 0;
    explicit AlignedBuffer(SINT n)
            : data(new (std::align_val_t(32)) CSAMPLE[static_cast<size_t>(n)]),
              size(n) {
    }
    ~AlignedBuffer() {
        ::operator delete[](data, std::align_val_t(32));
    }
    AlignedBuffer(const AlignedBuffer&) = delete;
    AlignedBuffer& operator=(const AlignedBuffer&) = delete;
};

float maxAbsDiff(const CSAMPLE* a, const CSAMPLE* b, SINT n) {
    float m = 0.0f;
    for (SINT i = 0; i < n; ++i) {
        m = std::max(m, std::abs(a[i] - b[i]));
    }
    return m;
}

void fillRandom(CSAMPLE* p, SINT n, unsigned seed) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    for (SINT i = 0; i < n; ++i) {
        p[i] = dist(rng);
    }
}

bool cpuHasAVX2() {
    return EngineStemMixer::hasAVX2();
}

} // namespace

TEST(EngineStemMixerTest, ReportsAVX2OnCapableCpu) {
    // i7-13620H: AVX2+FMA presentes. Solo informativo fuera de x86.
    printf("[EngineStemMixer] hasAVX2=%d\n", cpuHasAVX2() ? 1 : 0);
#if defined(__x86_64__)
    EXPECT_TRUE(cpuHasAVX2());
#endif
}

TEST(EngineStemMixerTest, BuffersAre32ByteAligned) {
    AlignedBuffer in(kFrames * 8), out(kFrames * 2);
    EXPECT_EQ(0u, reinterpret_cast<uintptr_t>(in.data) % 32);
    EXPECT_EQ(0u, reinterpret_cast<uintptr_t>(out.data) % 32);
}

TEST(EngineStemMixerTest, ConstantGainMatchesScalar) {
    if (!cpuHasAVX2()) {
        GTEST_SKIP() << "sin AVX2: dispatch == escalar por construccion";
    }
    for (SINT frames : {SINT{1}, SINT{3}, SINT{7}, SINT{8}, SINT{1023}, kFrames}) {
        AlignedBuffer in(frames * 8), avx(frames * 2), ref(frames * 2);
        fillRandom(in.data, frames * 8, 0x1234 + static_cast<unsigned>(frames));
        const GainArray gains = {0.8f, 0.0f, 1.0f, 0.5f};
        EngineStemMixer::processAVX2(avx.data, in.data, gains, frames);
        EngineStemMixer::processScalar(ref.data, in.data, gains, frames);
        const float d = maxAbsDiff(avx.data, ref.data, frames * 2);
        printf("[EngineStemMixer] frames=%td constant maxDiff=%.3g\n",
                frames, d);
        EXPECT_LE(d, kTol) << "frames=" << frames;
    }
}

TEST(EngineStemMixerTest, RampedGainMatchesScalar) {
    if (!cpuHasAVX2()) {
        GTEST_SKIP() << "sin AVX2";
    }
    for (SINT frames : {SINT{1}, SINT{2}, SINT{33}, SINT{1023}, kFrames}) {
        AlignedBuffer in(frames * 8), avx(frames * 2), ref(frames * 2);
        fillRandom(in.data, frames * 8, 0xABCD + static_cast<unsigned>(frames));
        const GainArray oldG = {1.0f, 1.0f, 1.0f, 1.0f};
        const GainArray newG = {0.0f, 0.7f, 0.25f, 1.0f};
        EngineStemMixer::processAVX2(avx.data, in.data, oldG, newG, frames);
        EngineStemMixer::processScalar(ref.data, in.data, oldG, newG, frames);
        const float d = maxAbsDiff(avx.data, ref.data, frames * 2);
        printf("[EngineStemMixer] frames=%td ramped maxDiff=%.3g\n",
                frames, d);
        EXPECT_LE(d, kTol) << "frames=" << frames;
    }
}

TEST(EngineStemMixerTest, NullTestPhaseCancellation) {
    // Null-test: AVX2 + (-escalar) debe anularse (tolerancia 1e-6).
    if (!cpuHasAVX2()) {
        GTEST_SKIP() << "sin AVX2";
    }
    AlignedBuffer in(kFrames * 8), avx(kFrames * 2), ref(kFrames * 2);
    fillRandom(in.data, kFrames * 8, 0x5EED);
    const GainArray oldG = {0.2f, 0.9f, 0.4f, 1.0f};
    const GainArray newG = {1.0f, 0.1f, 0.6f, 0.3f};
    EngineStemMixer::processAVX2(avx.data, in.data, oldG, newG, kFrames);
    EngineStemMixer::processScalar(ref.data, in.data, oldG, newG, kFrames);
    float peak = 0.0f;
    for (SINT i = 0; i < kFrames * 2; ++i) {
        peak = std::max(peak, std::abs(avx.data[i] + (-ref.data[i])));
    }
    printf("[EngineStemMixer] null-test peak=%.3g\n", peak);
    EXPECT_LE(peak, kTol);
}

TEST(EngineStemMixerTest, UnityGainEqualsPlainSumAndMuteIsSilent) {
    AlignedBuffer in(kFrames * 8), out(kFrames * 2);
    fillRandom(in.data, kFrames * 8, 42);
    const GainArray unity = {1.0f, 1.0f, 1.0f, 1.0f};
    EngineStemMixer::processScalar(out.data, in.data, unity, kFrames);
    for (SINT i = 0; i < kFrames; ++i) {
        const float expectL = in.data[i * 8] + in.data[i * 8 + 2] +
                in.data[i * 8 + 4] + in.data[i * 8 + 6];
        const float expectR = in.data[i * 8 + 1] + in.data[i * 8 + 3] +
                in.data[i * 8 + 5] + in.data[i * 8 + 7];
        EXPECT_NEAR(out.data[i * 2], expectL, kTol);
        EXPECT_NEAR(out.data[i * 2 + 1], expectR, kTol);
    }
    const GainArray mute = {0.0f, 0.0f, 0.0f, 0.0f};
    EngineStemMixer::processScalar(out.data, in.data, mute, kFrames);
    for (SINT i = 0; i < kFrames * 2; ++i) {
        EXPECT_EQ(out.data[i], 0.0f);
    }
}

TEST(EngineStemMixerTest, RampIsClickFree) {
    // Entrada DC + rampa 0->1: salida monotona, salto max = pendiente.
    AlignedBuffer in(kFrames * 8), out(kFrames * 2);
    std::fill_n(in.data, kFrames * 8, 0.5f);
    const GainArray oldG = {0.0f, 0.0f, 0.0f, 0.0f};
    const GainArray newG = {1.0f, 1.0f, 1.0f, 1.0f};
    EngineStemMixer::processScalar(out.data, in.data, oldG, newG, kFrames);
    const float maxStep = 2.0f / static_cast<float>(kFrames) + 1e-6f;
    for (SINT i = 1; i < kFrames; ++i) {
        EXPECT_GE(out.data[i * 2], out.data[(i - 1) * 2] - 1e-7f);
        EXPECT_LE(std::abs(out.data[i * 2] - out.data[(i - 1) * 2]), maxStep);
    }
    // Ultimo frame cae exactamente en new: 0.5*4 = 2.0.
    EXPECT_NEAR(out.data[(kFrames - 1) * 2], 2.0f, 1e-5f);
    EXPECT_NEAR(out.data[(kFrames - 1) * 2 + 1], 2.0f, 1e-5f);
}

TEST(EngineStemMixerTest, DispatchMatchesScalar) {
    AlignedBuffer in(kFrames * 8), out(kFrames * 2), ref(kFrames * 2);
    fillRandom(in.data, kFrames * 8, 7);
    const GainArray oldG = {1.0f, 0.5f, 0.0f, 0.8f};
    const GainArray newG = {0.3f, 1.0f, 0.9f, 0.0f};
    EngineStemMixer::process(out.data, in.data, oldG, newG, kFrames);
    EngineStemMixer::processScalar(ref.data, in.data, oldG, newG, kFrames);
    EXPECT_LE(maxAbsDiff(out.data, ref.data, kFrames * 2), kTol);
}

TEST(EngineStemMixerTest, ZeroFramesIsNoOp) {
    AlignedBuffer in(8), out(2);
    const GainArray g = {1.0f, 1.0f, 1.0f, 1.0f};
    out.data[0] = 123.0f;
    out.data[1] = 456.0f;
    EngineStemMixer::processScalar(out.data, in.data, g, g, 0);
    EngineStemMixer::process(out.data, in.data, g, g, 0);
    if (cpuHasAVX2()) {
        EngineStemMixer::processAVX2(out.data, in.data, g, g, 0);
    }
    EXPECT_EQ(out.data[0], 123.0f);
    EXPECT_EQ(out.data[1], 456.0f);
}

TEST(EngineStemMixerTest, BenchAVX2AtLeast2xScalar) {
    if (!cpuHasAVX2()) {
        GTEST_SKIP() << "sin AVX2";
    }
    AlignedBuffer in(kFrames * 8), out(kFrames * 2);
    fillRandom(in.data, kFrames * 8, 0xBEEC);
    const GainArray constG = {0.9f, 0.4f, 1.0f, 0.6f};
    const GainArray oldG = {0.9f, 0.4f, 1.0f, 0.6f};
    const GainArray newG = {0.1f, 1.0f, 0.5f, 0.8f};

    constexpr int kWarmup = 300;
    constexpr int kIters = 3000;
    constexpr int kRepeats = 9;
    // Las llamadas son opacas entre TUs (mixer va en mixxx-lib): el
    // compilador no puede eliminarlas (sin riesgo de DCE).
    for (int i = 0; i < kWarmup; ++i) {
        EngineStemMixer::processAVX2(out.data, in.data, oldG, newG, kFrames);
        EngineStemMixer::processScalar(out.data, in.data, oldG, newG, kFrames);
    }
    auto bench = [&](bool avx, bool ramp) {
        std::vector<double> times;
        times.reserve(kRepeats);
        for (int r = 0; r < kRepeats; ++r) {
            const auto t0 = std::chrono::steady_clock::now();
            for (int i = 0; i < kIters; ++i) {
                if (avx) {
                    if (ramp) {
                        EngineStemMixer::processAVX2(
                                out.data, in.data, oldG, newG, kFrames);
                    } else {
                        EngineStemMixer::processAVX2(
                                out.data, in.data, constG, kFrames);
                    }
                } else {
                    if (ramp) {
                        EngineStemMixer::processScalar(
                                out.data, in.data, oldG, newG, kFrames);
                    } else {
                        EngineStemMixer::processScalar(
                                out.data, in.data, constG, kFrames);
                    }
                }
            }
            const auto t1 = std::chrono::steady_clock::now();
            times.push_back(
                    std::chrono::duration<double, std::nano>(t1 - t0).count() /
                    kIters);
        }
        std::sort(times.begin(), times.end());
        return times[kRepeats / 2]; // mediana ns/call
    };
    double checksum = 0.0;
    for (SINT i = 0; i < kFrames * 2; ++i) {
        checksum += out.data[i];
    }
    printf("[EngineStemMixer] bench checksum=%f\n", checksum);
    const double nsAvxU = bench(true, false);
    const double nsScalarU = bench(false, false);
    const double speedupU = nsScalarU / nsAvxU;
    printf("[EngineStemMixer] bench uniform frames=%td avx=%.1f ns scalar=%.1f ns speedup=%.2fx\n",
            kFrames, nsAvxU, nsScalarU, speedupU);
    const double nsAvxR = bench(true, true);
    const double nsScalarR = bench(false, true);
    const double speedupR = nsScalarR / nsAvxR;
    printf("[EngineStemMixer] bench ramped frames=%td avx=%.1f ns scalar=%.1f ns speedup=%.2fx\n",
            kFrames, nsAvxR, nsScalarR, speedupR);
#if defined(__OPTIMIZE__) || (defined(_MSC_VER) && !defined(_DEBUG))
    // El criterio >=2x aplica al codigo optimizado (el audio thread real
    // siempre compila con -O3 salvo OPTIMIZE=off). Sin optimizar (-O0)
    // el bench solo informa: ese codegen no es representativo de RT.
    EXPECT_GE(speedupU, 2.0);
    EXPECT_GE(speedupR, 2.0);
#else
    printf("[EngineStemMixer] sin optimizar: asserts de speedup omitidos\n");
#endif
}
