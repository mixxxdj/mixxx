#include "visualizer/visualizerwindow.h"

#include <gtest/gtest.h>

namespace {

using mixxx::VisualizerWindow;

TEST(VisualizerWindowTest, combinedCoverIsAsLargeAsItIsDrawn) {
    // The cover takes 80 % of the height: 864 pixels in full HD and 1728
    // pixels in 4K. The size is rounded up to the next step of 256 pixels.
    EXPECT_EQ(1024, VisualizerWindow::combinedCoverSize(QSize(1920, 1080), 1.0));
    EXPECT_EQ(1792, VisualizerWindow::combinedCoverSize(QSize(3840, 2160), 1.0));
    // A portrait window is limited by its width.
    EXPECT_EQ(1024, VisualizerWindow::combinedCoverSize(QSize(1080, 1920), 1.0));
}

TEST(VisualizerWindowTest, combinedCoverCountsDevicePixels) {
    EXPECT_EQ(1792, VisualizerWindow::combinedCoverSize(QSize(1920, 1080), 2.0));
    EXPECT_EQ(1536, VisualizerWindow::combinedCoverSize(QSize(1920, 1080), 1.5));
}

TEST(VisualizerWindowTest, combinedCoverSizeIsLimited) {
    // Not larger than the covers themselves.
    EXPECT_EQ(2048, VisualizerWindow::combinedCoverSize(QSize(7680, 4320), 1.0));
    EXPECT_EQ(256, VisualizerWindow::combinedCoverSize(QSize(100, 100), 1.0));
    EXPECT_EQ(256, VisualizerWindow::combinedCoverSize(QSize(0, 0), 1.0));
}

} // namespace
