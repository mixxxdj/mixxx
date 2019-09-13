#include "util/cache.h"

#include <gtest/gtest.h>

#include <QtDebug>

namespace {

class CacheTest : public testing::Test {
};

TEST_F(CacheTest, InvalidCacheKey) {
    EXPECT_FALSE(mixxx::isValidCacheKey(
            mixxx::invalidCacheKey()));
}

TEST_F(CacheTest, InvalidCacheKeyEmptyByteArray) {
    EXPECT_FALSE(mixxx::isValidCacheKey(
            mixxx::calculateCacheKey(QByteArray())));
    EXPECT_FALSE(mixxx::isValidCacheKey(
            mixxx::calculateCacheKey("")));
    EXPECT_FALSE(mixxx::isValidCacheKey(
            mixxx::calculateCacheKey("\0")));
}

TEST_F(CacheTest, ValidCacheKey) {
    EXPECT_TRUE(mixxx::isValidCacheKey(
            mixxx::calculateCacheKey(QByteArrayLiteral("test"))));
}

TEST_F(CacheTest, ValidCacheKeySingleZeroAscii) {
    EXPECT_TRUE(mixxx::isValidCacheKey(
            mixxx::calculateCacheKey(QByteArray("0"))));
}

TEST_F(CacheTest, ValidCacheKeySingleZeroByte) {
    EXPECT_TRUE(mixxx::isValidCacheKey(
            mixxx::calculateCacheKey(QByteArray("\0", 1))));
}

} // anonymous namespace
