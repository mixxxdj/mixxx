#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "library/autodj/smart/beatmatch.h"
#include "library/autodj/smart/vocalmap.h"

namespace {

constexpr double kRate = 50.0; // values per second

// A song of `seconds` with the vocal part at `level` inside the ranges.
std::vector<float> vocalLevels(double seconds,
        const std::vector<std::pair<double, double>>& sung,
        float level = 0.5f,
        float background = 0.02f) {
    std::vector<float> v(static_cast<std::size_t>(seconds * kRate), background);
    for (const auto& [a, b] : sung) {
        for (auto i = static_cast<std::size_t>(a * kRate);
                i < static_cast<std::size_t>(b * kRate) && i < v.size();
                ++i) {
            v[i] = level;
        }
    }
    return v;
}

} // namespace

TEST(VocalMapTest, AnInstrumentalHasNoSinging) {
    const auto v = vocalLevels(200.0, {}, 0.5f, 0.01f);
    EXPECT_TRUE(vocalmap::find(v, {}, kRate).empty());
}

TEST(VocalMapTest, FindsTheVerses) {
    const auto v = vocalLevels(180.0, {{30.0, 70.0}, {100.0, 140.0}});
    const auto s = vocalmap::find(v, {}, kRate);
    ASSERT_EQ(s.size(), 2u);
    EXPECT_NEAR(s[0].startSec, 30.0, 0.5);
    EXPECT_NEAR(s[0].endSec, 70.0, 0.5);
    EXPECT_NEAR(s[1].startSec, 100.0, 0.5);
    EXPECT_NEAR(s[1].endSec, 140.0, 0.5);
}

TEST(VocalMapTest, BreathsStayOneSection) {
    // Lines of 4 s with 0.8 s breaths in between.
    std::vector<std::pair<double, double>> lines;
    for (double t = 20.0; t < 60.0; t += 4.8) {
        lines.push_back({t, t + 4.0});
    }
    const auto s = vocalmap::find(vocalLevels(120.0, lines), {}, kRate);
    ASSERT_EQ(s.size(), 1u);
    EXPECT_NEAR(s[0].startSec, 20.0, 0.5);
}

TEST(VocalMapTest, IgnoresShortBleedAndBandLeakage) {
    // A real verse, a 0.5 s blip, and a stretch where the vocal part only
    // carries a little of a loud band (an instrument leaking into it).
    auto v = vocalLevels(150.0, {{20.0, 50.0}, {80.0, 80.5}}, 0.6f);
    std::vector<float> band(v.size(), 0.3f);
    for (std::size_t i = static_cast<std::size_t>(100 * kRate);
            i < static_cast<std::size_t>(120 * kRate);
            ++i) {
        v[i] = 0.25f;
        band[i] = 3.0f;
    }
    const auto s = vocalmap::find(v, band, kRate);
    ASSERT_EQ(s.size(), 1u);
    EXPECT_NEAR(s[0].startSec, 20.0, 0.5);
}

TEST(VocalMapTest, QuietMomentsAndNextSinging) {
    const std::vector<vocalmap::Section> s{{10.0, 20.0}, {25.0, 40.0}};
    EXPECT_TRUE(vocalmap::singsBetween(s, 15.0, 16.0));
    EXPECT_FALSE(vocalmap::singsBetween(s, 20.5, 24.5));
    EXPECT_DOUBLE_EQ(vocalmap::quietMoment(s, 12.0, 30.0), 20.0);
    EXPECT_DOUBLE_EQ(vocalmap::quietMoment(s, 21.0, 30.0), 21.0);
    EXPECT_DOUBLE_EQ(vocalmap::quietMoment(s, 26.0, 35.0), -1.0);
    EXPECT_DOUBLE_EQ(vocalmap::nextSinging(s, 0.0), 10.0);
    EXPECT_DOUBLE_EQ(vocalmap::nextSinging(s, 21.0), 25.0);
    EXPECT_DOUBLE_EQ(vocalmap::nextSinging(s, 41.0), -1.0);
}

TEST(VocalMapTest, VocalsUntouchedWhenNobodySings) {
    const beatmatch::VocalPlan plan{false, false, 0.5};
    for (double p = 0.0; p <= 1.0001; p += 0.05) {
        const auto b = beatmatch::stemBlend(p, plan);
        EXPECT_DOUBLE_EQ(b.fromVocals, b.fromInstrumental) << p;
        EXPECT_DOUBLE_EQ(b.toVocals, b.toInstrumental) << p;
    }
}

TEST(VocalMapTest, BothSingHandOverAtTheEndOfALine) {
    const beatmatch::VocalPlan plan{true, true, 0.7};
    for (double p = 0.0; p <= 1.0001; p += 0.01) {
        const auto b = beatmatch::stemBlend(p, plan);
        EXPECT_TRUE(b.fromVocals < 1e-9 || b.toVocals < 1e-9) << p;
    }
    EXPECT_NEAR(beatmatch::stemBlend(0.4, plan).fromVocals, 1.0, 1e-9); // still singing
    EXPECT_NEAR(beatmatch::stemBlend(0.71, plan).fromVocals, 0.0, 1e-9);
    EXPECT_GT(beatmatch::stemBlend(0.8, plan).toVocals, 0.5);
    EXPECT_NEAR(beatmatch::stemBlend(1.0, plan).toVocals, 1.0, 1e-9);
}

TEST(VocalMapTest, OnlyTheNewSongSingsOverTheOldInstrumental) {
    const beatmatch::VocalPlan plan{false, true, 0.5};
    const auto b = beatmatch::stemBlend(0.3, plan);
    EXPECT_DOUBLE_EQ(b.toVocals, b.toInstrumental);
    EXPECT_GT(b.toVocals, 0.3);
}
