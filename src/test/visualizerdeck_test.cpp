#include "visualizer/visualizerdeck.h"

#include <gtest/gtest.h>

#include <QImage>
#include <numbers>

namespace {

using mixxx::VisualizerDeck;
using namespace std::chrono_literals;

TEST(VisualizerDeckTest, bandAmountIsZeroWithoutIntensity) {
    EXPECT_FLOAT_EQ(0.0f, VisualizerDeck::bandAmount(0.0, 1.0f));
    EXPECT_FLOAT_EQ(0.0f, VisualizerDeck::bandAmount(0.0, 0.5f));
}

TEST(VisualizerDeckTest, bandAmountIsZeroForSilentBand) {
    EXPECT_FLOAT_EQ(0.0f, VisualizerDeck::bandAmount(1.0, 0.0f));
    // What leaks into a killed band must not show either.
    EXPECT_FLOAT_EQ(0.0f, VisualizerDeck::bandAmount(1.0, 0.05f));
}

TEST(VisualizerDeckTest, bandAmountIsLimitedByIntensity) {
    EXPECT_FLOAT_EQ(1.0f, VisualizerDeck::bandAmount(1.0, 1.0f));
    EXPECT_FLOAT_EQ(0.25f, VisualizerDeck::bandAmount(0.25, 1.0f));
    EXPECT_LE(VisualizerDeck::bandAmount(0.25, 0.7f), 0.25f);
}

TEST(VisualizerDeckTest, bandAmountGrowsWithLevel) {
    EXPECT_LT(VisualizerDeck::bandAmount(1.0, 0.3f), VisualizerDeck::bandAmount(1.0, 0.6f));
    EXPECT_LT(VisualizerDeck::bandAmount(1.0, 0.6f), VisualizerDeck::bandAmount(1.0, 0.9f));
}

TEST(VisualizerDeckTest, bandAmountStaysInRange) {
    EXPECT_FLOAT_EQ(1.0f, VisualizerDeck::bandAmount(2.0, 3.0f));
    EXPECT_FLOAT_EQ(0.0f, VisualizerDeck::bandAmount(-1.0, 1.0f));
}

TEST(VisualizerDeckTest, tempoPhaseAdvancesWithTempo) {
    // A cycle of 8 beats takes 4 s at 120 BPM.
    const float quarterCycle = static_cast<float>(std::numbers::pi / 2);
    EXPECT_NEAR(quarterCycle, VisualizerDeck::advancedTempoPhase(0.0f, 120.0, 8.0, 1s), 1e-5);
    EXPECT_NEAR(quarterCycle / 2,
            VisualizerDeck::advancedTempoPhase(0.0f, 60.0, 8.0, 1s),
            1e-5);
    EXPECT_NEAR(quarterCycle * 2,
            VisualizerDeck::advancedTempoPhase(quarterCycle, 120.0, 8.0, 1s),
            1e-5);
}

TEST(VisualizerDeckTest, tempoPhaseCompletesACycleAfterTheGivenBeats) {
    // A cycle of 4 beats takes 2 s at 120 BPM.
    EXPECT_NEAR(std::numbers::pi,
            VisualizerDeck::advancedTempoPhase(0.0f, 120.0, 4.0, 1s),
            1e-5);
    EXPECT_NEAR(std::numbers::pi / 2,
            VisualizerDeck::advancedTempoPhase(0.0f, 120.0, 4.0, 500ms),
            1e-5);
}

TEST(VisualizerDeckTest, tempoPhaseWrapsAround) {
    const float phase = VisualizerDeck::advancedTempoPhase(0.0f, 120.0, 8.0, 5s);
    EXPECT_NEAR(std::numbers::pi / 2, phase, 1e-4);
    EXPECT_NEAR(std::numbers::pi / 2,
            VisualizerDeck::advancedTempoPhase(0.0f, 120.0, 4.0, 2500ms),
            1e-4);
}

TEST(VisualizerDeckTest, tempoPhaseKeepsMovingWithoutTempo) {
    EXPECT_FLOAT_EQ(VisualizerDeck::advancedTempoPhase(0.0f, 120.0, 8.0, 1s),
            VisualizerDeck::advancedTempoPhase(0.0f, 0.0, 8.0, 1s));
}

TEST(VisualizerDeckTest, preparedCoverIsScaledDownAndBlurred) {
    QImage cover(5000, 2500, QImage::Format_RGB32);
    cover.fill(Qt::darkCyan);
    const VisualizerDeck::PreparedCover prepared = VisualizerDeck::prepareCover(7, cover);
    EXPECT_EQ(7, prepared.m_request);
    EXPECT_EQ(QSize(2048, 1024), prepared.m_cover.size());
    EXPECT_EQ(QSize(64, 32), prepared.m_blurredCover.size());
}

TEST(VisualizerDeckTest, preparedCoverKeepsSmallPictures) {
    QImage cover(300, 200, QImage::Format_RGB32);
    cover.fill(Qt::darkCyan);
    const VisualizerDeck::PreparedCover prepared = VisualizerDeck::prepareCover(1, cover);
    EXPECT_EQ(cover.size(), prepared.m_cover.size());
    EXPECT_FALSE(prepared.m_blurredCover.isNull());
}

TEST(VisualizerDeckTest, missingCoverIsReplacedByTheDefaultCoverInFullSize) {
    const VisualizerDeck::PreparedCover prepared = VisualizerDeck::prepareCover(1, QImage());
    EXPECT_EQ(QSize(2048, 2048), prepared.m_cover.size());
    EXPECT_FALSE(prepared.m_blurredCover.isNull());
}

} // namespace
