#include <gtest/gtest.h>

#include <vector>

#include "analyzer/analyzerhotcues.h"

namespace {

using mixxx::hotcues::placeCues;

// beats at a loudness and bass level in db
void section(std::vector<double>* pLoud, std::vector<double>* pBass, int beats, double loud, double bass) {
    pLoud->insert(pLoud->end(), beats, loud);
    pBass->insert(pBass->end(), beats, bass);
}

TEST(AnalyzerHotcuesTest, CuesAtSectionChanges) {
    std::vector<double> loud;
    std::vector<double> bass;
    section(&loud, &bass, 2, -100, -100); // silence, bars start on beat 2
    section(&loud, &bass, 32, -20, -40);  // intro
    section(&loud, &bass, 64, -10, -12);  // drop
    section(&loud, &bass, 32, -18, -45);  // breakdown
    section(&loud, &bass, 32, -10, -12);  // drop

    const auto cues = placeCues(loud, bass);
    ASSERT_EQ(4, cues.size());
    EXPECT_EQ(2, cues[0].beat);
    EXPECT_EQ("Start", cues[0].label);
    EXPECT_EQ(34, cues[1].beat);
    EXPECT_EQ("Drop", cues[1].label);
    EXPECT_EQ(98, cues[2].beat);
    EXPECT_EQ("Break", cues[2].label);
    EXPECT_EQ(130, cues[3].beat);
    EXPECT_EQ("Drop", cues[3].label);
}

TEST(AnalyzerHotcuesTest, SteadyTrackOnlyGetsStart) {
    std::vector<double> loud;
    std::vector<double> bass;
    section(&loud, &bass, 1, -100, -100);
    section(&loud, &bass, 200, -10, -12);

    const auto cues = placeCues(loud, bass);
    ASSERT_EQ(1, cues.size());
    EXPECT_EQ(1, cues[0].beat);
}

TEST(AnalyzerHotcuesTest, SilenceGetsNone) {
    EXPECT_TRUE(placeCues(std::vector<double>(64, -100), std::vector<double>(64, -100)).empty());
}

} // namespace
