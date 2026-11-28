#include "visualizer/visualizercontrols.h"

#include <gtest/gtest.h>

#include <limits>

namespace {

using mixxx::VisualizerControls;

TEST(VisualizerControlsTest, styleRoundTrip) {
    for (VisualizerControls::Style style : {VisualizerControls::Style::CoverDistortion,
                 VisualizerControls::Style::CustomImage}) {
        EXPECT_EQ(style,
                VisualizerControls::styleFromControlValue(
                        VisualizerControls::controlValueFromStyle(style)));
    }
}

TEST(VisualizerControlsTest, controlValuesAreStable) {
    // The values are saved in the user settings.
    EXPECT_DOUBLE_EQ(0.0,
            VisualizerControls::controlValueFromStyle(
                    VisualizerControls::Style::CoverDistortion));
    EXPECT_DOUBLE_EQ(1.0,
            VisualizerControls::controlValueFromStyle(VisualizerControls::Style::CustomImage));
}

TEST(VisualizerControlsTest, unknownValuesGiveTheDefaultStyle) {
    for (double value : {-1.0, 2.0, 1000.0, std::numeric_limits<double>::quiet_NaN()}) {
        EXPECT_EQ(VisualizerControls::Style::CoverDistortion,
                VisualizerControls::styleFromControlValue(value));
    }
}

} // namespace
