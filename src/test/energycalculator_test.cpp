#include <gtest/gtest.h>

#include <cmath>
#include <random>
#include <vector>

#include "analyzer/energycalculator.h"

namespace {

constexpr double kRate = 44100.0;
constexpr double kPi = 3.14159265358979323846;

// Builds `seconds` of stereo audio from a mono generator f(t in seconds).
template<typename F>
std::vector<float> stereo(double seconds, F f) {
    const auto frames = static_cast<std::size_t>(seconds * kRate);
    std::vector<float> out(frames * 2);
    for (std::size_t i = 0; i < frames; ++i) {
        const auto v = static_cast<float>(f(i / kRate));
        out[2 * i] = v;
        out[2 * i + 1] = v;
    }
    return out;
}

bool analyse(const std::vector<float>& samples, EnergyCalculator::Result* pResult, int channels = 2) {
    EnergyCalculator calc(kRate, channels);
    // Feed in chunks like the analyzer thread does.
    constexpr std::size_t kChunk = 4096 * 2;
    for (std::size_t i = 0; i < samples.size(); i += kChunk) {
        const std::size_t n = std::min(kChunk, samples.size() - i);
        calc.process(samples.data() + i, static_cast<std::int64_t>(n));
    }
    return calc.finish(pResult);
}

} // namespace

TEST(EnergyCalculatorTest, SilenceHasNoResult) {
    EnergyCalculator::Result r;
    EXPECT_FALSE(analyse(stereo(20, [](double) { return 0.0; }), &r));
}

TEST(EnergyCalculatorTest, TooShortHasNoResult) {
    std::mt19937 rng(1);
    std::uniform_real_distribution<double> noise(-0.3, 0.3);
    EnergyCalculator::Result r;
    EXPECT_FALSE(analyse(stereo(3, [&](double) { return noise(rng); }), &r));
}

TEST(EnergyCalculatorTest, CombineStaysInRangeAndRises) {
    EXPECT_DOUBLE_EQ(1.0, EnergyCalculator::combine(0, 0, 0));
    EXPECT_DOUBLE_EQ(10.0, EnergyCalculator::combine(1, 1, 1));
    EXPECT_DOUBLE_EQ(10.0, EnergyCalculator::combine(5, 5, 5)); // clamped
    EXPECT_LT(EnergyCalculator::combine(0.2, 0.5, 0.5), EnergyCalculator::combine(0.8, 0.5, 0.5));
    EXPECT_LT(EnergyCalculator::combine(0.5, 0.2, 0.5), EnergyCalculator::combine(0.5, 0.8, 0.5));
    EXPECT_LT(EnergyCalculator::combine(0.5, 0.5, 0.2), EnergyCalculator::combine(0.5, 0.5, 0.8));
}

TEST(EnergyCalculatorTest, CountsOnsetsOfAClickTrack) {
    // 4 hits per second (quarter notes at 240 BPM) over a quiet noise floor.
    std::mt19937 rng(2);
    std::uniform_real_distribution<double> noise(-0.01, 0.01);
    const auto audio = stereo(30, [&](double t) {
        const double sinceHit = std::fmod(t, 0.25);
        const double hit = sinceHit < 0.03 ? 0.6 * std::exp(-sinceHit * 80.0) : 0.0;
        return noise(rng) + hit * std::sin(2 * kPi * 100 * t);
    });
    EnergyCalculator::Result r;
    ASSERT_TRUE(analyse(audio, &r));
    EXPECT_NEAR(4.0, r.onsetsPerSec, 0.5);
}

TEST(EnergyCalculatorTest, NoiseIsBrighterThanBass) {
    std::mt19937 rng(3);
    std::uniform_real_distribution<double> noise(-0.3, 0.3);
    EnergyCalculator::Result white;
    EnergyCalculator::Result bass;
    ASSERT_TRUE(analyse(stereo(20, [&](double) { return noise(rng); }), &white));
    ASSERT_TRUE(analyse(stereo(20, [](double t) { return 0.3 * std::sin(2 * kPi * 60 * t); }), &bass));
    EXPECT_GT(white.brightRatio, 0.3);
    EXPECT_LT(bass.brightRatio, 0.01);
    EXPECT_GT(bass.bassRatio, 0.6); // gentle 12 dB/oct filters: 60 Hz keeps ~74%
    EXPECT_GT(white.brightness01, bass.brightness01);
}

TEST(EnergyCalculatorTest, LoudBusyBrightBeatsQuietCalm) {
    std::mt19937 rng(4);
    std::uniform_real_distribution<double> noise(-1.0, 1.0);
    // "Peak-time": loud, bright noise with a hit every 0.25 s.
    const auto peak = stereo(30, [&](double t) {
        const double sinceHit = std::fmod(t, 0.25);
        const double hit = sinceHit < 0.03 ? std::exp(-sinceHit * 80.0) : 0.0;
        return 0.15 * noise(rng) + 0.7 * hit * std::sin(2 * kPi * 60 * t);
    });
    // "Ambient": a quiet, soft chord with no hits.
    const auto ambient = stereo(30, [](double t) {
        return 0.02 * (std::sin(2 * kPi * 220 * t) + std::sin(2 * kPi * 277 * t));
    });
    EnergyCalculator::Result rPeak;
    EnergyCalculator::Result rAmbient;
    ASSERT_TRUE(analyse(peak, &rPeak));
    ASSERT_TRUE(analyse(ambient, &rAmbient));
    EXPECT_GT(rPeak.energy, rAmbient.energy + 3.0)
            << "peak " << rPeak.energy << " ambient " << rAmbient.energy;
    EXPECT_GE(rAmbient.energy, 1.0);
    EXPECT_LE(rPeak.energy, 10.0);
}

TEST(EnergyCalculatorTest, MonoAndStereoAgree) {
    std::mt19937 rng(5);
    std::uniform_real_distribution<double> noise(-0.2, 0.2);
    std::vector<float> mono(static_cast<std::size_t>(20 * kRate));
    for (auto& s : mono) {
        s = static_cast<float>(noise(rng));
    }
    std::vector<float> both(mono.size() * 2);
    for (std::size_t i = 0; i < mono.size(); ++i) {
        both[2 * i] = both[2 * i + 1] = mono[i];
    }
    EnergyCalculator::Result rMono;
    EnergyCalculator::Result rStereo;
    ASSERT_TRUE(analyse(mono, &rMono, 1));
    ASSERT_TRUE(analyse(both, &rStereo, 2));
    EXPECT_NEAR(rMono.energy, rStereo.energy, 1e-6);
}

TEST(EnergyCalculatorTest, FindsTheBodyBetweenQuietIntroAndFadeOut) {
    // 16 s quiet pad intro, 60 s loud and busy, then a 20 s fade-out.
    std::mt19937 rng(6);
    std::uniform_real_distribution<double> noise(-1.0, 1.0);
    const auto audio = stereo(96, [&](double t) {
        if (t < 16.0) {
            return 0.02 * std::sin(2 * kPi * 220 * t); // pad, no drums
        }
        const double sinceHit = std::fmod(t, 0.5);
        const double hit = sinceHit < 0.03 ? std::exp(-sinceHit * 80.0) : 0.0;
        double level = 1.0;
        if (t > 76.0) {
            level = std::max(0.0, 1.0 - (t - 76.0) / 20.0); // linear fade-out
        }
        return level * (0.15 * noise(rng) + 0.7 * hit * std::sin(2 * kPi * 60 * t));
    });
    EnergyCalculator::Result r;
    ASSERT_TRUE(analyse(audio, &r));
    EXPECT_NEAR(16.0, r.bodyStartSec, 1.5) << r.bodyStartSec;
    // Body end uses -3 dB (0.71 of the amplitude): about 6 s into the 20 s
    // fade (~82 s), well before the track has faded to nothing.
    EXPECT_GT(r.bodyEndSec, 79.0) << r.bodyEndSec;
    EXPECT_LT(r.bodyEndSec, 85.0) << r.bodyEndSec;
}

TEST(EnergyCalculatorTest, WholeTrackIsBodyWhenLevelIsSteady) {
    std::mt19937 rng(7);
    std::uniform_real_distribution<double> noise(-0.3, 0.3);
    EnergyCalculator::Result r;
    ASSERT_TRUE(analyse(stereo(40, [&](double) { return noise(rng); }), &r));
    EXPECT_NEAR(0.0, r.bodyStartSec, 1e-9);
    EXPECT_NEAR(40.0, r.bodyEndSec, 1.0);
}

TEST(EnergyCalculatorTest, FindsTheDrumsAfterALoudIntroWithoutBass) {
    // 20.3 s of loud synth noise with no bass (as loud as the rest), then
    // the same noise plus a bass kick every 0.5 s. Loudness alone would say
    // the body starts at 0; the bass says 20.3 s (the first kick).
    std::mt19937 rng(8);
    std::uniform_real_distribution<double> noise(-1.0, 1.0);
    double hp = 0.0;
    const auto audio = stereo(80, [&](double t) {
        // crude high-pass: noise minus its smoothed version
        const double n = noise(rng);
        hp += 0.05 * (n - hp);
        const double synth = 0.25 * (n - hp);
        if (t < 20.3) {
            return synth;
        }
        const double sinceHit = std::fmod(t - 20.3, 0.5);
        const double hit = sinceHit < 0.08 ? std::exp(-sinceHit * 30.0) : 0.0;
        return synth + 0.6 * hit * std::sin(2 * kPi * 55 * t);
    });
    EnergyCalculator::Result r;
    ASSERT_TRUE(analyse(audio, &r));
    EXPECT_NEAR(20.3, r.bodyStartSec, 0.1) << r.bodyStartSec;
}

namespace {

// A drum loop: a kick on every beat (beat `beatSec`, first at `firstSec`)
// and a hat on every eighth note. `beatAt(n)` can bend the timing.
// `offBeatBass` adds a bass note half-way between the kicks (common in EBM).
template<typename BeatAt>
std::vector<float> drumLoop(double seconds,
        BeatAt beatAt,
        bool offBeatBass,
        double doubleKickFromSec = -1.0) {
    std::mt19937 rng(7);
    std::uniform_real_distribution<double> noise(-1.0, 1.0);
    const auto frames = static_cast<std::size_t>(seconds * kRate);
    std::vector<float> out(frames);
    int beat = 0;
    for (std::size_t i = 0; i < frames; ++i) {
        const double t = i / kRate;
        while (beatAt(beat + 1) <= t) {
            ++beat;
        }
        const double start = beatAt(beat);
        const double length = beatAt(beat + 1) - start;
        const double sinceKick = t - start;
        double v = 0.0;
        if (sinceKick >= 0.0) {
            // A double kick: a short tick, then the full kick 60 ms later.
            constexpr double kSecondTick = 0.06;
            if (doubleKickFromSec >= 0.0 && t >= doubleKickFromSec) {
                v += 0.6 * std::sin(2 * kPi * 55 * sinceKick) * std::exp(-sinceKick / 0.015);
                if (sinceKick >= kSecondTick) {
                    const double since = sinceKick - kSecondTick;
                    v += 0.6 * std::sin(2 * kPi * 55 * since) * std::exp(-since / 0.06);
                }
            } else {
                v += 0.6 * std::sin(2 * kPi * 55 * sinceKick) * std::exp(-sinceKick / 0.06);
            }
            const double sinceHat = std::fmod(sinceKick, 0.5 * length);
            v += 0.08 * noise(rng) * std::exp(-sinceHat / 0.02);
            if (offBeatBass && sinceKick >= 0.5 * length) {
                const double sinceBass = sinceKick - 0.5 * length;
                v += 0.8 * std::sin(2 * kPi * 80 * sinceBass) * std::exp(-sinceBass / 0.1);
            }
        }
        out[i] = static_cast<float>(v);
    }
    return out;
}

double gridDrift(const std::vector<float>& mono, const phrasealign::Grid& grid) {
    EnergyCalculator calc(kRate, 1);
    calc.process(mono.data(), static_cast<std::int64_t>(mono.size()));
    EnergyCalculator::Result r;
    if (!calc.finish(&r)) {
        return -2.0;
    }
    return calc.gridDriftBeats(grid, r.bodyStartSec, r.bodyEndSec);
}

double gridDrift(const std::vector<float>& mono, double firstBeatSec, double beatSec) {
    return gridDrift(mono, phrasealign::Grid{firstBeatSec, beatSec});
}

} // namespace

TEST(EnergyCalculatorTest, GridThatFitsDoesNotDrift) {
    const auto loop = drumLoop(200, [](int n) { return 0.3 + 0.5 * n; }, false);
    const double drift = gridDrift(loop, 0.3, 0.5);
    EXPECT_GE(drift, 0.0);
    EXPECT_LT(drift, 0.05);
}

TEST(EnergyCalculatorTest, OffBeatBassIsFine) {
    // Where the hits fall in the beat does not matter, only that it stays.
    const auto loop = drumLoop(200, [](int n) { return 0.3 + 0.5 * n; }, true);
    const double drift = gridDrift(loop, 0.3, 0.5);
    EXPECT_GE(drift, 0.0);
    EXPECT_LT(drift, 0.05);
    // Nor does a grid that is off by a steady amount (a wrong phase cannot
    // be told from an off-beat pattern).
    EXPECT_LT(gridDrift(loop, 0.4, 0.5), 0.05);
}

TEST(EnergyCalculatorTest, DoubleKicksDoNotLookLikeDrift) {
    // Single kicks for the first half, double kicks after (a new beat
    // style): the pattern changes, but the grid still fits.
    const auto loop = drumLoop(
            200, [](int n) { return 0.3 + 0.5 * n; }, false, 100.0);
    const double drift = gridDrift(loop, 0.3, 0.5);
    EXPECT_GE(drift, 0.0);
    EXPECT_LT(drift, 0.05);
}

TEST(EnergyCalculatorTest, GridWithTheWrongTempoDrifts) {
    const auto loop = drumLoop(200, [](int n) { return 0.3 + 0.5 * n; }, false);
    // 0.1% and 1% too slow or fast.
    EXPECT_GT(gridDrift(loop, 0.3, 0.5 * 1.001), EnergyCalculator::kGridMaxDriftBeats);
    EXPECT_GT(gridDrift(loop, 0.3, 0.5 / 1.001), EnergyCalculator::kGridMaxDriftBeats);
    EXPECT_GT(gridDrift(loop, 0.3, 0.5 * 1.01), EnergyCalculator::kGridMaxDriftBeats);
}

TEST(EnergyCalculatorTest, DrummerWhoSlowsDownDrifts) {
    // Steady for the first half, then 1% slower: no steady grid fits.
    const auto loop = drumLoop(200,
            [](int n) { return n < 200 ? 0.3 + 0.5 * n : 100.3 + 0.505 * (n - 200); },
            false);
    EXPECT_GT(gridDrift(loop, 0.3, 0.5), EnergyCalculator::kGridMaxDriftBeats);
}

TEST(EnergyCalculatorTest, OneOddStretchIsNotDrift) {
    // A steady track with one 8-second stretch (a breakdown) whose hits
    // fall half a beat later, right where the check looks at the start.
    const auto beatAt = [](int n) {
        const double t = 0.3 + 0.5 * n;
        return n >= 16 && n < 32 ? t + 0.25 : t;
    };
    const auto loop = drumLoop(200, beatAt, false);
    const double drift = gridDrift(loop, 0.3, 0.5);
    EXPECT_GE(drift, 0.0);
    EXPECT_LT(drift, EnergyCalculator::kGridMaxDriftBeats);
}

TEST(EnergyCalculatorTest, BeatMapThatBendsWithTheDrummerDoesNotDrift) {
    // A live drummer: the tempo wanders slowly up and down by about 2%.
    const auto beatAt = [](int n) {
        return 0.3 + 0.5 * n + 1.6 * std::sin(n / 40.0);
    };
    const auto loop = drumLoop(200, beatAt, false);
    // No steady grid fits...
    EXPECT_GT(gridDrift(loop, 0.3, 0.5), EnergyCalculator::kGridMaxDriftBeats);
    // ...but Mixxx's beat map, which follows every beat, does.
    std::vector<double> beats;
    for (int n = 0; beatAt(n) < 200.0; ++n) {
        beats.push_back(beatAt(n));
    }
    const double drift = gridDrift(loop, phrasealign::Grid::fromBeats(beats));
    EXPECT_GE(drift, 0.0);
    EXPECT_LT(drift, EnergyCalculator::kGridMaxDriftBeats);
}

TEST(EnergyCalculatorTest, NoBeatMeansCannotTell) {
    std::mt19937 rng(3);
    std::uniform_real_distribution<double> noise(-0.3, 0.3);
    std::vector<float> hiss(static_cast<std::size_t>(200 * kRate));
    for (float& s : hiss) {
        s = static_cast<float>(noise(rng));
    }
    EXPECT_DOUBLE_EQ(-1.0, gridDrift(hiss, 0.3, 0.5));
    // Too short to judge: the body is shorter than 64 beats.
    const auto shortLoop = drumLoop(25, [](int n) { return 0.3 + 0.5 * n; }, false);
    EXPECT_DOUBLE_EQ(-1.0, gridDrift(shortLoop, 0.3, 0.5));
}
