#include <gtest/gtest.h>

#include "qml/qmldprutils.h"

namespace {

using mixxx::qml::RenderInvalidationReason;
using mixxx::qml::RenderInvalidationState;

TEST(QmlRenderInvalidationTest, InitialFullSurfaceDominatesDirtyRegion) {
    RenderInvalidationState state(true);
    state.invalidate(RenderInvalidationReason::Viewport,
            QRegion(QRect(1, 2, 3, 4)));

    EXPECT_TRUE(state.fullSurface);
    EXPECT_TRUE(state.dirtyRegion.isEmpty());
    const auto pending = state.take();
    EXPECT_TRUE(pending.fullSurface);
    EXPECT_FALSE(state.fullSurface);
    EXPECT_TRUE(state.dirtyRegion.isEmpty());
}

TEST(QmlRenderInvalidationTest, FullSurfaceDominatesDirtyRegions) {
    RenderInvalidationState state;
    state.invalidate(RenderInvalidationReason::Viewport,
            QRegion(QRect(1, 2, 3, 4)));
    state.invalidate(RenderInvalidationReason::FullSurface);
    state.invalidate(RenderInvalidationReason::Viewport,
            QRegion(QRect(10, 20, 3, 4)));

    EXPECT_TRUE(state.fullSurface);
    EXPECT_TRUE(state.dirtyRegion.isEmpty());
    EXPECT_EQ(RenderInvalidationReason::FullSurface, state.invalidationReason);
}

TEST(QmlRenderInvalidationTest, UnionsMultipleDirtyRegions) {
    RenderInvalidationState state;
    const QRegion first(QRect(1, 2, 3, 4));
    const QRegion second(QRect(10, 20, 5, 6));
    state.invalidate(RenderInvalidationReason::Viewport, first);
    state.invalidate(RenderInvalidationReason::Viewport, second);

    QRegion expected = first;
    expected += second;
    EXPECT_FALSE(state.fullSurface);
    EXPECT_TRUE(state.dirtyRegion == expected);
    EXPECT_EQ(RenderInvalidationReason::Viewport, state.invalidationReason);
}

TEST(QmlRenderInvalidationTest, UnknownAndEmptyRegionsPromoteToFullSurface) {
    RenderInvalidationState state;
    state.invalidate(RenderInvalidationReason::Viewport);
    EXPECT_TRUE(state.fullSurface);
    EXPECT_EQ(RenderInvalidationReason::FullSurface, state.invalidationReason);

    state.reset();
    state.invalidate(RenderInvalidationReason::Unknown,
            QRegion(QRect(1, 2, 3, 4)));
    EXPECT_TRUE(state.fullSurface);
    EXPECT_TRUE(state.dirtyRegion.isEmpty());
    EXPECT_EQ(RenderInvalidationReason::Unknown, state.invalidationReason);
}

TEST(QmlRenderInvalidationTest, TakeResetsStateForTheNextFrame) {
    RenderInvalidationState state;
    state.invalidate(RenderInvalidationReason::Viewport,
            QRegion(QRect(1, 2, 3, 4)));
    const auto pending = state.take();

    EXPECT_FALSE(pending.fullSurface);
    EXPECT_FALSE(pending.dirtyRegion.isEmpty());
    EXPECT_FALSE(state.fullSurface);
    EXPECT_TRUE(state.dirtyRegion.isEmpty());
    EXPECT_EQ(RenderInvalidationReason::Unknown, state.invalidationReason);

    state.invalidate(RenderInvalidationReason::Viewport,
            QRegion(QRect(10, 20, 3, 4)));
    EXPECT_FALSE(state.fullSurface);
    EXPECT_TRUE(state.dirtyRegion == QRegion(QRect(10, 20, 3, 4)));
}

TEST(QmlRenderInvalidationTest, ClipsDirtyRegionsAndPromotesEmptyClipToFull) {
    RenderInvalidationState state;
    state.invalidate(RenderInvalidationReason::Viewport,
            QRegion(QRect(-2, -2, 8, 8)));
    state.clipTo(QRegion(QRect(0, 0, 10, 10)));

    EXPECT_FALSE(state.fullSurface);
    EXPECT_TRUE(state.dirtyRegion == QRegion(QRect(0, 0, 6, 6)));

    state.reset();
    state.invalidate(RenderInvalidationReason::Viewport,
            QRegion(QRect(20, 20, 4, 4)));
    state.clipTo(QRegion(QRect(0, 0, 10, 10)));
    EXPECT_TRUE(state.fullSurface);
    EXPECT_TRUE(state.dirtyRegion.isEmpty());
    EXPECT_EQ(RenderInvalidationReason::FullSurface, state.invalidationReason);
}

TEST(QmlDprUtilsTest, ConvertsLogicalSizeUsingCeiling) {
    EXPECT_EQ(QSize(150, 75),
            mixxx::qml::physicalSizeForLogicalSize(QSize(100, 50), 1.5));
    EXPECT_EQ(QSize(4, 5),
            mixxx::qml::physicalSizeForLogicalSize(QSize(3, 4), 1.25));
}

TEST(QmlDprUtilsTest, RepairsInvalidDpr) {
    EXPECT_EQ(QSize(3, 4),
            mixxx::qml::physicalSizeForLogicalSize(QSize(3, 4), 0.0));
}

} // namespace
