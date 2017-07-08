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

TEST_F(IndexRangeTest, contains) {
    EXPECT_FALSE(IndexRange().contains(0));
    EXPECT_FALSE(IndexRange().contains(-1));
    EXPECT_FALSE(IndexRange().contains(2));
    EXPECT_TRUE(IndexRange::between(1, -2).contains(0));
    EXPECT_TRUE(IndexRange::between(1, -2).contains(1));
    EXPECT_FALSE(IndexRange::between(1, -2).contains(-2));
    EXPECT_TRUE(IndexRange::between(-2, 5).contains(-2));
    EXPECT_TRUE(IndexRange::between(-2, 5).contains(0));
    EXPECT_TRUE(IndexRange::between(-2, 5).contains(4));
    EXPECT_FALSE(IndexRange::between(-2, 5).contains(5));
}

TEST_F(IndexRangeTest, clamp) {
    EXPECT_EQ(-1, IndexRange::between(-1, 1).clamp(-2));
    EXPECT_EQ(-1, IndexRange::between(-1, 1).clamp(-1));
    EXPECT_EQ(0, IndexRange::between(-1, 1).clamp(0));
    EXPECT_EQ(1, IndexRange::between(-1, 1).clamp(1));
    EXPECT_EQ(1, IndexRange::between(-1, 1).clamp(2));
    EXPECT_EQ(-1, IndexRange::between(-1, -1).clamp(-2));
    EXPECT_EQ(-1, IndexRange::between(-1, -1).clamp(2));
    EXPECT_EQ(0, IndexRange::between(0, 0).clamp(-2));
    EXPECT_EQ(0, IndexRange::between(0, 0).clamp(2));
    EXPECT_EQ(1, IndexRange::between(1, 1).clamp(-2));
    EXPECT_EQ(1, IndexRange::between(1, 1).clamp(2));
}

TEST_F(IndexRangeTest, headtail) {
    EXPECT_EQ(-1, IndexRange::between(-1, 1).head());
    EXPECT_EQ(1, IndexRange::between(-1, 1).tail());
    EXPECT_EQ(1, IndexRange::between(1, -1).head());
    EXPECT_EQ(-1, IndexRange::between(1, -1).tail());
    EXPECT_EQ(-1, IndexRange::between(-1, -1).head());
    EXPECT_EQ(-1, IndexRange::between(-1, -1).tail());
    EXPECT_EQ(0, IndexRange::between(0, 0).head());
    EXPECT_EQ(0, IndexRange::between(0, 0).tail());
    EXPECT_EQ(1, IndexRange::between(1, 1).head());
    EXPECT_EQ(1, IndexRange::between(1, 1).tail());
}

TEST_F(IndexRangeTest, dropHead) {
    auto r1 = IndexRange::between(-4, 9);
    r1.dropHead(3);
    EXPECT_EQ(IndexRange::between(-1, 9), r1);
    r1.dropHead(0);
    EXPECT_EQ(IndexRange::between(-1, 9), r1);
    r1.dropHead(r1.length());
    EXPECT_TRUE(r1.empty());

    auto r2 = IndexRange::between(9, -4);
    r2.dropHead(3);
    EXPECT_EQ(IndexRange::between(6, -4), r2);
    r2.dropHead(0);
    EXPECT_EQ(IndexRange::between(6, -4), r2);
    r2.dropHead(r2.length());
    EXPECT_TRUE(r2.empty());
}

TEST_F(IndexRangeTest, splitHead) {
    auto r1 = IndexRange::between(-4, 9);
    EXPECT_EQ(IndexRange::between(-4, -1), r1.splitHead(3));
    EXPECT_EQ(IndexRange::between(-1, 9), r1);
    EXPECT_TRUE(r1.splitHead(0).empty());
    EXPECT_EQ(IndexRange::between(-1, 9), r1);
    EXPECT_EQ(IndexRange::between(-1, 9), r1.splitHead(r1.length()));
    EXPECT_TRUE(r1.empty());

    auto r2 = IndexRange::between(9, -4);
    EXPECT_EQ(IndexRange::between(9, 6), r2.splitHead(3));
    EXPECT_EQ(IndexRange::between(6, -4), r2);
    EXPECT_TRUE(r2.splitHead(0).empty());
    EXPECT_EQ(IndexRange::between(6, -4), r2);
    EXPECT_EQ(IndexRange::between(6, -4), r2.splitHead(r2.length()));
    EXPECT_TRUE(r2.empty());
}

TEST_F(IndexRangeTest, dropTail) {
    auto r1 = IndexRange::between(-4, 9);
    r1.dropTail(3);
    EXPECT_EQ(IndexRange::between(-4, 6), r1);
    r1.dropTail(0);
    EXPECT_EQ(IndexRange::between(-4, 6), r1);
    r1.dropTail(r1.length());
    EXPECT_TRUE(r1.empty());

    auto r2 = IndexRange::between(9, -4);
    r2.dropTail(3);
    EXPECT_EQ(IndexRange::between(9, -1), r2);
    r2.dropTail(0);
    EXPECT_EQ(IndexRange::between(9, -1), r2);
    r2.dropTail(r2.length());
    EXPECT_TRUE(r2.empty());
}

TEST_F(IndexRangeTest, splitTail) {
    auto r1 = IndexRange::between(-4, 9);
    EXPECT_EQ(IndexRange::between(6, 9), r1.splitTail(3));
    EXPECT_EQ(IndexRange::between(-4, 6), r1);
    EXPECT_TRUE(r1.splitTail(0).empty());
    EXPECT_EQ(IndexRange::between(-4, 6), r1);
    EXPECT_EQ(IndexRange::between(-4, 6), r1.splitTail(r1.length()));
    EXPECT_TRUE(r1.empty());

    auto r2 = IndexRange::between(9, -4);
    EXPECT_EQ(IndexRange::between(-1, -4), r2.splitTail(3));
    EXPECT_EQ(IndexRange::between(9, -1), r2);
    EXPECT_TRUE(r2.splitTail(0).empty());
    EXPECT_EQ(IndexRange::between(9, -1), r2);
    EXPECT_EQ(IndexRange::between(9, -1), r2.splitTail(r2.length()));
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
    EXPECT_FALSE(IndexRange::between(-1, 3) == reverse(IndexRange::between(-1, 3)));
    EXPECT_TRUE(IndexRange::between(-1, 3) != reverse(IndexRange::between(-1, 3)));
}

TEST_F(IndexRangeTest, lessOrEqual) {
    EXPECT_TRUE(IndexRange() <= IndexRange::between(-2, 2));
    EXPECT_TRUE(IndexRange() <= IndexRange::between(2, -1));
    EXPECT_TRUE(IndexRange::between(-2, 1) <= IndexRange::between(-2, 2));
    EXPECT_TRUE(IndexRange::between(1, -2) <= IndexRange::between(2, -2));
    EXPECT_FALSE(IndexRange::between(-2, 1) <= IndexRange::between(-1, 2));
    EXPECT_FALSE(IndexRange::between(1, -2) <= IndexRange::between(2, -1));
    EXPECT_FALSE(IndexRange::between(-2, 1) <= IndexRange::between(0, 1));
    EXPECT_FALSE(IndexRange::between(-2, 1) <= IndexRange::between(1, 2));
    EXPECT_FALSE(IndexRange::between(-2, 1) <= IndexRange::between(1, 1));
    EXPECT_TRUE(IndexRange::between(3, 3) <= IndexRange::between(-2, 1));
    EXPECT_TRUE(IndexRange::between(3, 3) <= IndexRange::between(1, -2));
}

TEST_F(IndexRangeTest, greaterOrEqual) {
    EXPECT_TRUE(IndexRange::between(-2, 2) >= IndexRange());
    EXPECT_TRUE(IndexRange::between(-2, 2) >= IndexRange::between(-2, 1));
    EXPECT_FALSE(IndexRange::between(-1, 2) >= IndexRange::between(-2, 1));
    EXPECT_FALSE(IndexRange::between(2, -1) >= IndexRange::between(1, -2));
    EXPECT_FALSE(IndexRange::between(0, 1) >= IndexRange::between(-2, 1));
    EXPECT_FALSE(IndexRange::between(1, 2) >= IndexRange::between(-2, 1));
    EXPECT_FALSE(IndexRange::between(1, 1) >= IndexRange::between(-2, 1));
    EXPECT_TRUE(IndexRange::between(-2, 1) >= IndexRange::between(3, 3));
    EXPECT_TRUE(IndexRange::between(1, -2) >= IndexRange::between(3, 3));
}

TEST_F(IndexRangeTest, reverse) {
    EXPECT_EQ(IndexRange(), reverse(IndexRange()));
    EXPECT_EQ(IndexRange::between(1, 1), reverse(IndexRange::between(1, 1)));
    EXPECT_EQ(IndexRange::between(-1, -1), reverse(IndexRange::between(-1, -1)));
    EXPECT_EQ(IndexRange::between(8, -5), reverse(IndexRange::between(-4, 9)));
    EXPECT_EQ(IndexRange::between(-9, -3), reverse(IndexRange::between(-4, -10)));
}

TEST_F(IndexRangeTest, intersect) {
    EXPECT_EQ(IndexRange::between(0, 0), intersect(IndexRange::between(-1, 0), IndexRange::between(0, 1)));
    EXPECT_EQ(IndexRange::between(-1, 1), intersect(IndexRange::between(-1, 1), IndexRange::between(-1, 1)));
    EXPECT_EQ(IndexRange::between(-1, 1), intersect(IndexRange::between(-2, 1), IndexRange::between(-1, 2)));
    EXPECT_TRUE(intersect(IndexRange::between(-2, -1), IndexRange::between(1, 2)).empty());
    EXPECT_EQ(IndexRange::between(0, 0), intersect(IndexRange::between(0, -1), IndexRange::between(1, 0)));
    EXPECT_EQ(IndexRange::between(1, -1), intersect(IndexRange::between(1, -1), IndexRange::between(1, -1)));
    EXPECT_EQ(IndexRange::between(1, -1), intersect(IndexRange::between(2, -1), IndexRange::between(1, -2)));
    EXPECT_TRUE(intersect(IndexRange::between(-1, -2), IndexRange::between(2, 1)).empty());
}

TEST_F(IndexRangeTest, join) {
    EXPECT_EQ(IndexRange::between(-1, 1), join(IndexRange::between(-1, 0), IndexRange::between(0, 1)));
    EXPECT_EQ(IndexRange::between(-1, 1), join(IndexRange::between(-1, 1), IndexRange::between(-1, 1)));
    EXPECT_EQ(IndexRange::between(-2, 2), join(IndexRange::between(-2, 1), IndexRange::between(-1, 2)));
    EXPECT_EQ(IndexRange::between(-2, 2), join(IndexRange::between(-2, -1), IndexRange::between(1, 2)));
    EXPECT_EQ(IndexRange::between(1, -1), join(IndexRange::between(0, -1), IndexRange::between(1, 0)));
    EXPECT_EQ(IndexRange::between(1, -1), join(IndexRange::between(1, -1), IndexRange::between(1, -1)));
    EXPECT_EQ(IndexRange::between(2, -2), join(IndexRange::between(1, -2), IndexRange::between(2, -1)));
    EXPECT_EQ(IndexRange::between(2, -2), join(IndexRange::between(-1, -2), IndexRange::between(2, 1)));
}


} // namespace mixxx
