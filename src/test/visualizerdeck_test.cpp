#include "visualizer/visualizerdeck.h"

#include <gtest/gtest.h>

#include <numbers>

namespace {

using mixxx::VisualizerDeck;

TEST(VisualizerDeckTest, bandAmountIsZeroWithoutIntensity) {
    EXPECT_FLOAT_EQ(0.0f, VisualizerDeck::bandAmount(0.0, 1.0f));
    EXPECT_FLOAT_EQ(0.0f, VisualizerDeck::bandAmount(0.0, 0.5f));
}

TEST(VisualizerDeckTest, bandAmountIsZeroForSilentBand) {
    EXPECT_FLOAT_EQ(0.0f, VisualizerDeck::bandAmount(1.0, 0.0f));
    // [AI-generated] What leaks into a killed band must not show either.
    // [/AI-generated]
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

TEST(VisualizerDeckTest, mixWeightOfSingleDeckIsFull) {
    EXPECT_FLOAT_EQ(1.0f, VisualizerDeck::mixWeight(0.2f, 0.2f));
}

TEST(VisualizerDeckTest, mixWeightsFollowLoudness) {
    EXPECT_FLOAT_EQ(0.5f, VisualizerDeck::mixWeight(0.2f, 0.4f));
    EXPECT_FLOAT_EQ(0.75f, VisualizerDeck::mixWeight(0.3f, 0.4f));
    EXPECT_FLOAT_EQ(0.25f, VisualizerDeck::mixWeight(0.1f, 0.4f));
}

TEST(VisualizerDeckTest, mixWeightFadesOutWithSilence) {
    EXPECT_FLOAT_EQ(0.0f, VisualizerDeck::mixWeight(0.0f, 0.0f));
    EXPECT_FLOAT_EQ(0.0f, VisualizerDeck::mixWeight(0.0f, 0.3f));
    EXPECT_LT(VisualizerDeck::mixWeight(0.001f, 0.001f), 1.0f);
}

TEST(VisualizerDeckTest, flowPhaseAdvancesWithTempo) {
    // [AI-generated] A flow cycle takes 8 beats, which is 4 s at 120 BPM.
    // [/AI-generated]
    const float quarterCycle = static_cast<float>(std::numbers::pi / 2);
    EXPECT_NEAR(quarterCycle, VisualizerDeck::advancedFlowPhase(0.0f, 120.0, 1.0), 1e-5);
    EXPECT_NEAR(quarterCycle / 2, VisualizerDeck::advancedFlowPhase(0.0f, 60.0, 1.0), 1e-5);
    EXPECT_NEAR(quarterCycle * 2,
            VisualizerDeck::advancedFlowPhase(quarterCycle, 120.0, 1.0),
            1e-5);
}

TEST(VisualizerDeckTest, flowPhaseWrapsAround) {
    const float phase = VisualizerDeck::advancedFlowPhase(0.0f, 120.0, 5.0);
    EXPECT_NEAR(std::numbers::pi / 2, phase, 1e-4);
}

TEST(VisualizerDeckTest, flowPhaseKeepsMovingWithoutTempo) {
    EXPECT_FLOAT_EQ(VisualizerDeck::advancedFlowPhase(0.0f, 120.0, 1.0),
            VisualizerDeck::advancedFlowPhase(0.0f, 0.0, 1.0));
}

} // namespace
