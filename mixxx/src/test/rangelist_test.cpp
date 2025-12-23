#include "util/rangelist.h"

#include <gtest/gtest.h>

#include <QString>
#include <algorithm>

#include "test/mixxxtest.h"

void roundTripTestStr(const QString& in, const QString* out = nullptr) {
    const QString roundTrip = mixxx::stringifyRangeList(mixxx::parseRangeList(in));
    if (out == nullptr) {
        EXPECT_QSTRING_EQ(in, roundTrip);
    } else {
        EXPECT_QSTRING_EQ(*out, roundTrip);
    }
}

void roundTripTestList(const QList<int>& in, const QList<int>* out = nullptr) {
    const QList<int> roundTrip = mixxx::parseRangeList(mixxx::stringifyRangeList(in));
    if (out == nullptr) {
        EXPECT_EQ(in, roundTrip);
    } else {
        EXPECT_EQ(*out, roundTrip);
    }
}

TEST(DisplayIntListTest, ListEmpty) {
    roundTripTestList({});
    roundTripTestStr("");
}

TEST(DisplayIntListTest, smallContinousRangeIsSeparate) {
    const QList<int> list = mixxx::parseRangeList(QStringLiteral("1 - 2"));
    EXPECT_EQ(list, QList({1, 2}));
    EXPECT_QSTRING_EQ("1, 2", mixxx::stringifyRangeList(list));
}

TEST(DisplayIntListTest, whiteSpaceAreIgnored) {
    EXPECT_EQ(QList<int>({1, 2, 3, 4}), mixxx::parseRangeList("   1,2  ,  3, 4"));
}

TEST(DisplayIntListTest, trailingCommaIsIgnored) {
    EXPECT_EQ(QList<int>({1}), mixxx::parseRangeList("1,"));
}
TEST(DisplayIntListTest, largeRangesAreExpanded) {
    EXPECT_EQ(QList<int>({1, 2, 3, 4, 5, 6, 7}), mixxx::parseRangeList("1 - 7"));
}

TEST(DisplayIntListTest, duplicateValuesAreIgnored) {
    EXPECT_EQ(QList<int>({1, 2, 3}), mixxx::parseRangeList("1, 1, 1, 1, 2, 2, 3"));
}

TEST(DisplayIntListTest, resultingListIsSortedAscending) {
    const auto list = mixxx::parseRangeList("4, 2, 3, 1, 6, 5");
    EXPECT_TRUE(std::is_sorted(list.cbegin(), list.cend()));
}
TEST(DisplayIntListTest, consequitiveValuesAreRanges) {
    EXPECT_QSTRING_EQ("1 - 4", mixxx::stringifyRangeList(QList<int>({1, 2, 3, 4})));
}
TEST(DisplayIntListTest, adjacentRangeAndLiteralGetsCollapsed) {
    EXPECT_EQ(QList<int>({1, 2, 3, 4, 5}), mixxx::parseRangeList("1, 2 - 4, 5"));
}
TEST(DisplayIntListTest, overLappingRangesGetUnionized) {
    EXPECT_EQ(QList<int>({1, 2, 3, 4}), mixxx::parseRangeList("1 - 3, 2 - 4"));
}
