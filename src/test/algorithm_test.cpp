#include "util/algorithm.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <type_traits>

namespace {

using ::testing::ElementsAre;

template<typename T, int N, typename T2>
auto find(T (&arr)[N], const T2& elem) {
    static_assert(std::is_convertible_v<T, T2>);
    return std::lower_bound(std::begin(arr), std::end(arr), elem);
}

template<typename T, int N>
bool is_sorted(T (&arr)[N]) {
    return std::is_sorted(std::cbegin(arr), std::cend(arr));
}

TEST(ReplaceSortedTest, replaceLastInPlace) {
    double arr[] = {1, 3, 8};
    ASSERT_TRUE(is_sorted(arr));
    double* evict = find(arr, 8);
    ASSERT_NE(evict, std::end(arr));
    double* newPosition = mixxx::replace_sorted(std::begin(arr), std::end(arr), 5, evict);
    EXPECT_TRUE(is_sorted(arr));
    EXPECT_THAT(arr, ElementsAre(1, 3, 5));
    EXPECT_EQ(newPosition, find(arr, 5));
}

TEST(ReplaceSortedTest, replaceLastReorder) {
    int arr[] = {1, 3, 4};
    ASSERT_TRUE(is_sorted(arr));
    int* evict = find(arr, 1);
    ASSERT_NE(evict, std::end(arr));
    int* newPosition = mixxx::replace_sorted(std::begin(arr), std::end(arr), 6, evict);
    EXPECT_TRUE(is_sorted(arr));
    EXPECT_THAT(arr, ElementsAre(3, 4, 6));
    EXPECT_EQ(newPosition, find(arr, 6));
}

TEST(ReplaceSortedTest, replaceMiddleReorder) {
    int arr[] = {1, 3, 4};
    ASSERT_TRUE(is_sorted(arr));
    int* evict = find(arr, 4);
    ASSERT_NE(evict, std::end(arr));
    int* newPosition = mixxx::replace_sorted(std::begin(arr), std::end(arr), 2, evict);
    EXPECT_TRUE(is_sorted(arr));
    EXPECT_THAT(arr, ElementsAre(1, 2, 3));
    EXPECT_EQ(newPosition, find(arr, 2));
}

} // namespace
