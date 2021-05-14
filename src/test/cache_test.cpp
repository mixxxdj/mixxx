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
            mixxx::cacheKeyFromMessageDigest(QByteArray())));
    EXPECT_FALSE(mixxx::isValidCacheKey(
            mixxx::cacheKeyFromMessageDigest("")));
    EXPECT_FALSE(mixxx::isValidCacheKey(
            mixxx::cacheKeyFromMessageDigest("\0")));
}

TEST_F(CacheTest, ValidCacheKey) {
    EXPECT_TRUE(mixxx::isValidCacheKey(
            mixxx::cacheKeyFromMessageDigest(QByteArrayLiteral("test"))));
}

TEST_F(CacheTest, ValidCacheKeySingleZeroAscii) {
    EXPECT_TRUE(mixxx::isValidCacheKey(
            mixxx::cacheKeyFromMessageDigest(QByteArray("0"))));
}

TEST_F(CacheTest, ValidCacheKeySingleZeroByte) {
    EXPECT_TRUE(mixxx::isValidCacheKey(
            mixxx::cacheKeyFromMessageDigest(QByteArray("\0", 1))));
}

} // anonymous namespace
