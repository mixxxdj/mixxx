#include <gtest/gtest.h>

#include "util/indexrange.h"

namespace mixxx {

class IndexRangeTest : public testing::Test {
};

TEST_F(IndexRangeTest, empty) {
    EXPECT_TRUE(IndexRange().empty());
    EXPECT_TRUE(IndexRange::between(-1, -1).empty());
    EXPECT_FALSE(IndexRange::between(0, -1).empty());
    EXPECT_TRUE(IndexRange::between(0, 0).empty());
    EXPECT_FALSE(IndexRange::between(1, 0).empty());
    EXPECT_TRUE(IndexRange::between(1, 1).empty());
    EXPECT_FALSE(IndexRange::between(1, -1).empty());
    EXPECT_FALSE(IndexRange::between(-1, 0).empty());
    EXPECT_FALSE(IndexRange::between(0, 1).empty());
    EXPECT_FALSE(IndexRange::between(-1, 1).empty());
}

TEST_F(IndexRangeTest, orientation) {
    EXPECT_EQ(IndexRange::Orientation::Empty, IndexRange().orientation());
    EXPECT_EQ(IndexRange::Orientation::Empty, IndexRange::between(1, 1).orientation());
    EXPECT_EQ(IndexRange::Orientation::Empty, IndexRange::between(-1, -1).orientation());
    EXPECT_EQ(IndexRange::Orientation::Forward, IndexRange::between(-4, 9).orientation());
    EXPECT_EQ(IndexRange::Orientation::Backward, IndexRange::between(-4, -10).orientation());
}

TEST_F(IndexRangeTest, length) {
    EXPECT_EQ(0, IndexRange().length());
    EXPECT_EQ(0, IndexRange::between(1, 1).length());
    EXPECT_EQ(0, IndexRange::between(-1, -1).length());
    EXPECT_EQ(1, IndexRange::between(2, 1).length());
    EXPECT_EQ(3, IndexRange::between(1, -2).length());
    EXPECT_EQ(1, IndexRange::between(-1, -2).length());
    EXPECT_EQ(1, IndexRange::between(0, 1).length());
    EXPECT_EQ(5, IndexRange::between(0, 5).length());
    EXPECT_EQ(6, IndexRange::between(1, 7).length());
    EXPECT_EQ(2, IndexRange::between(-1, 1).length());
    EXPECT_EQ(7, IndexRange::between(-2, 5).length());
    EXPECT_EQ(1, IndexRange::between(-2, -1).length());
}

TEST_F(IndexRangeTest, containsIndex) {
    EXPECT_FALSE(IndexRange().containsIndex(0));
    EXPECT_FALSE(IndexRange().containsIndex(-1));
    EXPECT_FALSE(IndexRange().containsIndex(2));
    EXPECT_TRUE(IndexRange::between(1, -2).containsIndex(0));
    EXPECT_TRUE(IndexRange::between(1, -2).containsIndex(1));
    EXPECT_FALSE(IndexRange::between(1, -2).containsIndex(-2));
    EXPECT_TRUE(IndexRange::between(-2, 5).containsIndex(-2));
    EXPECT_TRUE(IndexRange::between(-2, 5).containsIndex(0));
    EXPECT_TRUE(IndexRange::between(-2, 5).containsIndex(4));
    EXPECT_FALSE(IndexRange::between(-2, 5).containsIndex(5));
}

TEST_F(IndexRangeTest, clampIndex) {
    EXPECT_EQ(-1, IndexRange::between(-1, 1).clampIndex(-2));
    EXPECT_EQ(-1, IndexRange::between(-1, 1).clampIndex(-1));
    EXPECT_EQ(0, IndexRange::between(-1, 1).clampIndex(0));
    EXPECT_EQ(1, IndexRange::between(-1, 1).clampIndex(1));
    EXPECT_EQ(1, IndexRange::between(-1, 1).clampIndex(2));
    EXPECT_EQ(-1, IndexRange::between(-1, -1).clampIndex(-2));
    EXPECT_EQ(-1, IndexRange::between(-1, -1).clampIndex(2));
    EXPECT_EQ(0, IndexRange::between(0, 0).clampIndex(-2));
    EXPECT_EQ(0, IndexRange::between(0, 0).clampIndex(2));
    EXPECT_EQ(1, IndexRange::between(1, 1).clampIndex(-2));
    EXPECT_EQ(1, IndexRange::between(1, 1).clampIndex(2));
}

TEST_F(IndexRangeTest, startend) {
    EXPECT_EQ(-1, IndexRange::between(-1, 1).start());
    EXPECT_EQ(1, IndexRange::between(-1, 1).end());
    EXPECT_EQ(1, IndexRange::between(1, -1).start());
    EXPECT_EQ(-1, IndexRange::between(1, -1).end());
    EXPECT_EQ(-1, IndexRange::between(-1, -1).start());
    EXPECT_EQ(-1, IndexRange::between(-1, -1).end());
    EXPECT_EQ(0, IndexRange::between(0, 0).start());
    EXPECT_EQ(0, IndexRange::between(0, 0).end());
    EXPECT_EQ(1, IndexRange::between(1, 1).start());
    EXPECT_EQ(1, IndexRange::between(1, 1).end());
}

TEST_F(IndexRangeTest, shrinkFront) {
    auto r1 = IndexRange::between(-4, 9);
    r1.shrinkFront(3);
    EXPECT_EQ(IndexRange::between(-1, 9), r1);
    r1.shrinkFront(0);
    EXPECT_EQ(IndexRange::between(-1, 9), r1);
    r1.shrinkFront(r1.length());
    EXPECT_TRUE(r1.empty());

    auto r2 = IndexRange::between(9, -4);
    r2.shrinkFront(3);
    EXPECT_EQ(IndexRange::between(6, -4), r2);
    r2.shrinkFront(0);
    EXPECT_EQ(IndexRange::between(6, -4), r2);
    r2.shrinkFront(r2.length());
    EXPECT_TRUE(r2.empty());
}

TEST_F(IndexRangeTest, splitAndShrinkFront) {
    auto r1 = IndexRange::between(-4, 9);
    EXPECT_EQ(IndexRange::between(-4, -1), r1.splitAndShrinkFront(3));
    EXPECT_EQ(IndexRange::between(-1, 9), r1);
    EXPECT_TRUE(r1.splitAndShrinkFront(0).empty());
    EXPECT_EQ(IndexRange::between(-1, 9), r1);
    EXPECT_EQ(IndexRange::between(-1, 9), r1.splitAndShrinkFront(r1.length()));
    EXPECT_TRUE(r1.empty());

    auto r2 = IndexRange::between(9, -4);
    EXPECT_EQ(IndexRange::between(9, 6), r2.splitAndShrinkFront(3));
    EXPECT_EQ(IndexRange::between(6, -4), r2);
    EXPECT_TRUE(r2.splitAndShrinkFront(0).empty());
    EXPECT_EQ(IndexRange::between(6, -4), r2);
    EXPECT_EQ(IndexRange::between(6, -4), r2.splitAndShrinkFront(r2.length()));
    EXPECT_TRUE(r2.empty());
}

TEST_F(IndexRangeTest, shrinkBack) {
    auto r1 = IndexRange::between(-4, 9);
    r1.shrinkBack(3);
    EXPECT_EQ(IndexRange::between(-4, 6), r1);
    r1.shrinkBack(0);
    EXPECT_EQ(IndexRange::between(-4, 6), r1);
    r1.shrinkBack(r1.length());
    EXPECT_TRUE(r1.empty());

    auto r2 = IndexRange::between(9, -4);
    r2.shrinkBack(3);
    EXPECT_EQ(IndexRange::between(9, -1), r2);
    r2.shrinkBack(0);
    EXPECT_EQ(IndexRange::between(9, -1), r2);
    r2.shrinkBack(r2.length());
    EXPECT_TRUE(r2.empty());
}

TEST_F(IndexRangeTest, splitAndShrinkBack) {
    auto r1 = IndexRange::between(-4, 9);
    EXPECT_EQ(IndexRange::between(6, 9), r1.splitAndShrinkBack(3));
    EXPECT_EQ(IndexRange::between(-4, 6), r1);
    EXPECT_TRUE(r1.splitAndShrinkBack(0).empty());
    EXPECT_EQ(IndexRange::between(-4, 6), r1);
    EXPECT_EQ(IndexRange::between(-4, 6), r1.splitAndShrinkBack(r1.length()));
    EXPECT_TRUE(r1.empty());

    auto r2 = IndexRange::between(9, -4);
    EXPECT_EQ(IndexRange::between(-1, -4), r2.splitAndShrinkBack(3));
    EXPECT_EQ(IndexRange::between(9, -1), r2);
    EXPECT_TRUE(r2.splitAndShrinkBack(0).empty());
    EXPECT_EQ(IndexRange::between(9, -1), r2);
    EXPECT_EQ(IndexRange::between(9, -1), r2.splitAndShrinkBack(r2.length()));
    EXPECT_TRUE(r2.empty());
}

TEST_F(IndexRangeTest, equal) {
    EXPECT_TRUE(IndexRange() == IndexRange());
    EXPECT_FALSE(IndexRange() != IndexRange());
    EXPECT_TRUE(IndexRange::between(3, 3) == IndexRange::between(3, 3));
    EXPECT_FALSE(IndexRange::between(3, 3) != IndexRange::between(3, 3));
    EXPECT_FALSE(IndexRange::between(2, 2) == IndexRange::between(3, 3));
    EXPECT_TRUE(IndexRange::between(2, 2) != IndexRange::between(3, 3));
    EXPECT_TRUE(IndexRange::forward(-1, 3) == IndexRange::between(-1, 2));
    EXPECT_FALSE(IndexRange::forward(-1, 3) != IndexRange::between(-1, 2));
    EXPECT_TRUE(IndexRange::backward(-1, 3) == IndexRange::between(-1, -4));
    EXPECT_FALSE(IndexRange::backward(-1, 3) != IndexRange::between(-1, -4));
    EXPECT_FALSE(IndexRange::between(0, 0) == IndexRange::between(1, 1));
    EXPECT_TRUE(IndexRange::between(0, 0) != IndexRange::between(1, 1));
}

TEST_F(IndexRangeTest, isSubrangeOf) {
    EXPECT_FALSE(IndexRange::between(3, 3).isSubrangeOf(IndexRange::between(-2, 2)));
    EXPECT_TRUE(IndexRange::between(0, 0).isSubrangeOf(IndexRange::between(2, -1)));
    EXPECT_TRUE(IndexRange::between(2, 2).isSubrangeOf(IndexRange::between(2, -1)));
    EXPECT_FALSE(IndexRange::between(-2, -2).isSubrangeOf(IndexRange::between(2, -1)));
    EXPECT_TRUE(IndexRange::between(-2, 1).isSubrangeOf(IndexRange::between(-2, 2)));
    EXPECT_TRUE(IndexRange::between(1, -2).isSubrangeOf(IndexRange::between(2, -2)));
    EXPECT_FALSE(IndexRange::between(-2, 1).isSubrangeOf(IndexRange::between(-1, 2)));
    EXPECT_FALSE(IndexRange::between(1, -2).isSubrangeOf(IndexRange::between(2, -1)));
    EXPECT_FALSE(IndexRange::between(-2, 1).isSubrangeOf(IndexRange::between(0, 1)));
    EXPECT_FALSE(IndexRange::between(-2, 1).isSubrangeOf(IndexRange::between(1, 2)));
    EXPECT_FALSE(IndexRange::between(-2, 1).isSubrangeOf(IndexRange::between(1, 1)));
    EXPECT_FALSE(IndexRange::between(3, 3).isSubrangeOf(IndexRange::between(-2, 1)));
    EXPECT_FALSE(IndexRange::between(3, 3).isSubrangeOf(IndexRange::between(1, -2)));
    EXPECT_TRUE(IndexRange::between(1, 1).isSubrangeOf(IndexRange::between(-2, 1)));
    EXPECT_TRUE(IndexRange::between(1, 1).isSubrangeOf(IndexRange::between(1, -2)));
}

TEST_F(IndexRangeTest, intersect2) {
    EXPECT_EQ(IndexRange::between(0, 0),
            *intersect2(IndexRange::between(-1, 0), IndexRange::between(0, 1)));
    EXPECT_EQ(IndexRange::between(-1, 1),
            *intersect2(
                    IndexRange::between(-1, 1), IndexRange::between(-1, 1)));
    EXPECT_EQ(IndexRange::between(-1, 1),
            *intersect2(
                    IndexRange::between(-2, 1), IndexRange::between(-1, 2)));
    EXPECT_EQ(std::nullopt, intersect2(IndexRange::between(-2, -1), IndexRange::between(1, 2)));
    EXPECT_EQ(IndexRange::between(0, 0),
            *intersect2(IndexRange::between(0, -1), IndexRange::between(1, 0)));
    EXPECT_EQ(IndexRange::between(1, -1),
            *intersect2(
                    IndexRange::between(1, -1), IndexRange::between(1, -1)));
    EXPECT_EQ(IndexRange::between(1, -1),
            *intersect2(
                    IndexRange::between(2, -1), IndexRange::between(1, -2)));
    EXPECT_EQ(std::nullopt, intersect2(IndexRange::between(-1, -2), IndexRange::between(2, 1)));
    EXPECT_EQ(std::nullopt, intersect2(IndexRange::between(1, 2), IndexRange::between(3, 4)));
    EXPECT_EQ(std::nullopt, intersect2(IndexRange::between(-1, -2), IndexRange::between(-3, -4)));
    EXPECT_EQ(std::nullopt, intersect2(IndexRange(), IndexRange::between(1, 2)));
    EXPECT_EQ(std::nullopt, intersect2(IndexRange(), IndexRange::between(-1, -2)));
    EXPECT_EQ(IndexRange::between(1, 1),
            *intersect2(IndexRange::between(1, 1), IndexRange::between(1, 1)));
    EXPECT_EQ(IndexRange::between(-1, -1),
            *intersect2(
                    IndexRange::between(-1, -1), IndexRange::between(-1, -1)));
    EXPECT_EQ(IndexRange::between(1, 1),
            *intersect2(IndexRange::between(0, 1), IndexRange::between(1, 1)));
    EXPECT_EQ(IndexRange::between(-1, -1),
            *intersect2(
                    IndexRange::between(0, -1), IndexRange::between(-1, -1)));
}

} // namespace mixxx
