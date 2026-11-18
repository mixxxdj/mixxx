#include "visualizer/visualizerlayout.h"

#include <gtest/gtest.h>

#include <QVector2D>

namespace {

using mixxx::VisualizerLayout;

constexpr float kTolerance = 0.002f;
const QSizeF kPictureSize(1000, 1000);

// [AI-generated] Returns the layout for decks that have been playing with
// the given shares for a long time. [/AI-generated]
VisualizerLayout settledLayout(const VisualizerLayout::PerDeck& shares) {
    VisualizerLayout layout;
    layout.update(shares, 100.0, kPictureSize);
    layout.update(shares, 100.0, kPictureSize);
    return layout;
}

float totalArea(const VisualizerLayout& layout) {
    float area = 0.0f;
    for (int deckIndex = 0; deckIndex < VisualizerLayout::kMaxDecks; ++deckIndex) {
        area += layout.cell(deckIndex).m_area;
    }
    return area;
}

float borderDistance(const VisualizerLayout::Cell& cell, int borderIndex, const QVector2D& p) {
    const QVector3D& border = cell.m_borders[borderIndex];
    return QVector2D::dotProduct(border.toVector2D(), p) + border.z();
}

// [AI-generated] Returns the index of the deck whose cell contains the
// position, or -1 if it is not well inside of any cell. [/AI-generated]
int deckAt(const VisualizerLayout& layout, const QVector2D& p) {
    for (int deckIndex = 0; deckIndex < VisualizerLayout::kMaxDecks; ++deckIndex) {
        const VisualizerLayout::Cell& cell = layout.cell(deckIndex);
        if (cell.m_area > 0.0f &&
                borderDistance(cell, 0, p) > 1.0f &&
                borderDistance(cell, 1, p) > 1.0f &&
                borderDistance(cell, 2, p) > 1.0f) {
            return deckIndex;
        }
    }
    return -1;
}

TEST(VisualizerLayoutTest, sharesAreNormalizedLoudness) {
    const VisualizerLayout::PerDeck shares =
            VisualizerLayout::updatedShares({0.2f, 0.6f, 0.0f, 0.0f}, {});
    EXPECT_FLOAT_EQ(0.25f, shares[0]);
    EXPECT_FLOAT_EQ(0.75f, shares[1]);
    EXPECT_FLOAT_EQ(0.0f, shares[2]);
    EXPECT_FLOAT_EQ(0.0f, shares[3]);
}

TEST(VisualizerLayoutTest, sharesAreKeptWhileSilent) {
    const VisualizerLayout::PerDeck previous = {0.25f, 0.75f, 0.0f, 0.0f};
    const VisualizerLayout::PerDeck shares =
            VisualizerLayout::updatedShares({0.0f, 0.0001f, 0.0f, 0.0f}, previous);
    EXPECT_EQ(previous, shares);
}

TEST(VisualizerLayoutTest, singleDeckFillsThePicture) {
    const VisualizerLayout layout = settledLayout({0.0f, 1.0f, 0.0f, 0.0f});
    EXPECT_NEAR(1.0f, layout.cell(1).m_area, kTolerance);
    EXPECT_EQ(1, deckAt(layout, QVector2D(-0.49f, -0.49f)));
    EXPECT_EQ(1, deckAt(layout, QVector2D(0.49f, 0.49f)));
    EXPECT_FLOAT_EQ(0.0f, layout.cell(0).m_area);
    EXPECT_FLOAT_EQ(0.0f, layout.cell(2).m_area);
    EXPECT_FLOAT_EQ(0.0f, layout.cell(3).m_area);
}

TEST(VisualizerLayoutTest, twoEqualDecksSplitThePictureLeftAndRight) {
    const VisualizerLayout layout = settledLayout({0.5f, 0.5f, 0.0f, 0.0f});
    EXPECT_NEAR(0.5f, layout.cell(0).m_area, kTolerance);
    EXPECT_NEAR(0.5f, layout.cell(1).m_area, kTolerance);
    EXPECT_EQ(0, deckAt(layout, QVector2D(-0.25f, -0.4f)));
    EXPECT_EQ(0, deckAt(layout, QVector2D(-0.01f, 0.4f)));
    EXPECT_EQ(1, deckAt(layout, QVector2D(0.25f, 0.4f)));
    EXPECT_EQ(1, deckAt(layout, QVector2D(0.01f, -0.4f)));
}

TEST(VisualizerLayoutTest, twoDecksGetAreaInProportionToTheirShares) {
    const VisualizerLayout layout = settledLayout({0.75f, 0.25f, 0.0f, 0.0f});
    EXPECT_NEAR(0.75f, layout.cell(0).m_area, kTolerance);
    EXPECT_NEAR(0.25f, layout.cell(1).m_area, kTolerance);
    // [AI-generated] The border has moved to the right by a quarter of the
    // picture. [/AI-generated]
    EXPECT_EQ(0, deckAt(layout, QVector2D(0.24f, 0.0f)));
    EXPECT_EQ(1, deckAt(layout, QVector2D(0.26f, 0.0f)));
}

TEST(VisualizerLayoutTest, anyTwoDecksSplitThePictureLeftAndRight) {
    const VisualizerLayout layout = settledLayout({0.0f, 0.5f, 0.0f, 0.5f});
    EXPECT_NEAR(0.5f, layout.cell(1).m_area, kTolerance);
    EXPECT_NEAR(0.5f, layout.cell(3).m_area, kTolerance);
    EXPECT_EQ(1, deckAt(layout, QVector2D(-0.25f, 0.0f)));
    EXPECT_EQ(3, deckAt(layout, QVector2D(0.25f, 0.0f)));
}

TEST(VisualizerLayoutTest, threeEqualDecksMeetInTheMiddle) {
    const float third = 1.0f / 3.0f;
    const VisualizerLayout layout = settledLayout({third, third, third, 0.0f});
    for (int deckIndex = 0; deckIndex < 3; ++deckIndex) {
        EXPECT_NEAR(third, layout.cell(deckIndex).m_area, kTolerance);
    }
    EXPECT_NEAR(1.0f, totalArea(layout), kTolerance);
    // [AI-generated] Two wedges are at the top and one is at the bottom.
    // [/AI-generated]
    EXPECT_EQ(0, deckAt(layout, QVector2D(-0.3f, -0.3f)));
    EXPECT_EQ(1, deckAt(layout, QVector2D(0.3f, -0.3f)));
    EXPECT_EQ(2, deckAt(layout, QVector2D(0.0f, 0.4f)));
    // [AI-generated] All three wedges reach the middle of the picture.
    // [/AI-generated]
    EXPECT_EQ(0, deckAt(layout, QVector2D(-0.06f, -0.06f)));
    EXPECT_EQ(1, deckAt(layout, QVector2D(0.06f, -0.06f)));
    EXPECT_EQ(2, deckAt(layout, QVector2D(0.0f, 0.1f)));
}

TEST(VisualizerLayoutTest, fourEqualDecksSplitThePictureIntoQuarters) {
    const VisualizerLayout layout = settledLayout({0.25f, 0.25f, 0.25f, 0.25f});
    for (int deckIndex = 0; deckIndex < 4; ++deckIndex) {
        EXPECT_NEAR(0.25f, layout.cell(deckIndex).m_area, kTolerance);
    }
    EXPECT_EQ(0, deckAt(layout, QVector2D(-0.01f, -0.01f)));
    EXPECT_EQ(0, deckAt(layout, QVector2D(-0.49f, -0.49f)));
    EXPECT_EQ(1, deckAt(layout, QVector2D(0.01f, -0.01f)));
    EXPECT_EQ(1, deckAt(layout, QVector2D(0.49f, -0.49f)));
    EXPECT_EQ(2, deckAt(layout, QVector2D(-0.01f, 0.01f)));
    EXPECT_EQ(2, deckAt(layout, QVector2D(-0.49f, 0.49f)));
    EXPECT_EQ(3, deckAt(layout, QVector2D(0.01f, 0.01f)));
    EXPECT_EQ(3, deckAt(layout, QVector2D(0.49f, 0.49f)));
}

TEST(VisualizerLayoutTest, threeAndFourDecksGetAreaInProportionToTheirShares) {
    const VisualizerLayout::PerDeck threeShares = {0.5f, 0.3f, 0.2f, 0.0f};
    const VisualizerLayout three = settledLayout(threeShares);
    const VisualizerLayout::PerDeck fourShares = {0.15f, 0.2f, 0.4f, 0.25f};
    const VisualizerLayout four = settledLayout(fourShares);
    for (int deckIndex = 0; deckIndex < VisualizerLayout::kMaxDecks; ++deckIndex) {
        EXPECT_NEAR(threeShares[deckIndex], three.cell(deckIndex).m_area, kTolerance);
        EXPECT_NEAR(fourShares[deckIndex], four.cell(deckIndex).m_area, kTolerance);
    }
}

TEST(VisualizerLayoutTest, areaFollowsChangingShares) {
    // [AI-generated] Unlike in the other tests the layout is updated frame
    // by frame here, as in the window. [/AI-generated]
    VisualizerLayout layout;
    for (int frame = 0; frame <= 120; ++frame) {
        const float share = 0.2f + 0.005f * static_cast<float>(frame);
        const VisualizerLayout::PerDeck shares = {
                share, (1.0f - share) / 2, (1.0f - share) / 2, 0.0f};
        layout.update(shares, 1.0 / 60, kPictureSize);
        EXPECT_NEAR(shares[0], layout.cell(0).m_area, kTolerance);
        EXPECT_NEAR(shares[1], layout.cell(1).m_area, kTolerance);
        EXPECT_NEAR(shares[2], layout.cell(2).m_area, kTolerance);
    }
}

TEST(VisualizerLayoutTest, barelyAudibleDeckHasNoCell) {
    const VisualizerLayout layout = settledLayout({0.985f, 0.015f, 0.0f, 0.0f});
    EXPECT_NEAR(1.0f, layout.cell(0).m_area, kTolerance);
    EXPECT_FLOAT_EQ(0.0f, layout.cell(1).m_area);
}

TEST(VisualizerLayoutTest, cellOfJoiningDeckGrowsOutOfNothing) {
    float previousArea = 0.0f;
    for (float share = 0.021f; share <= 0.09f; share += 0.004f) {
        const VisualizerLayout layout = settledLayout({1.0f - share, share, 0.0f, 0.0f});
        const float area = layout.cell(1).m_area;
        EXPECT_GE(area, previousArea);
        // [AI-generated] The cell must not appear with a jump.
        // [/AI-generated]
        EXPECT_LT(area - previousArea, 0.012f);
        EXPECT_NEAR(1.0f, totalArea(layout), kTolerance);
        previousArea = area;
    }
    EXPECT_LT(settledLayout({0.979f, 0.021f, 0.0f, 0.0f}).cell(1).m_area, 0.001f);
    // [AI-generated] From here on the area is the share of the deck.
    // [/AI-generated]
    EXPECT_NEAR(0.09f, settledLayout({0.91f, 0.09f, 0.0f, 0.0f}).cell(1).m_area, kTolerance);
}

TEST(VisualizerLayoutTest, thirdAndFourthDeckJoinWithSmallCells) {
    const VisualizerLayout three = settledLayout({0.49f, 0.489f, 0.021f, 0.0f});
    EXPECT_LT(three.cell(2).m_area, 0.01f);
    EXPECT_NEAR(1.0f, totalArea(three), kTolerance);

    const VisualizerLayout four = settledLayout({0.33f, 0.33f, 0.319f, 0.021f});
    EXPECT_LT(four.cell(3).m_area, 0.01f);
    EXPECT_NEAR(1.0f, totalArea(four), kTolerance);
}

TEST(VisualizerLayoutTest, bordersMeasureTheDistanceInPixels) {
    const VisualizerLayout layout = settledLayout({0.5f, 0.5f, 0.0f, 0.0f});
    // [AI-generated] The border is the vertical line through the center of
    // the 1000 pixels wide picture. [/AI-generated]
    EXPECT_NEAR(250.0f, borderDistance(layout.cell(0), 0, QVector2D(-0.25f, 0.3f)), 0.5f);
    EXPECT_NEAR(-250.0f, borderDistance(layout.cell(0), 0, QVector2D(0.25f, -0.3f)), 0.5f);
    EXPECT_NEAR(250.0f, borderDistance(layout.cell(1), 0, QVector2D(0.25f, 0.0f)), 0.5f);
    // [AI-generated] The unused borders do not cut anything off.
    // [/AI-generated]
    EXPECT_GT(borderDistance(layout.cell(0), 1, QVector2D(0.5f, 0.5f)), 1000.0f);
    EXPECT_GT(borderDistance(layout.cell(0), 2, QVector2D(-0.5f, -0.5f)), 1000.0f);
}

} // namespace
