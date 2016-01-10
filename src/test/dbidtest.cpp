#include <gtest/gtest.h>

#include <QtDebug>

#include "util/dbid.h"

namespace {

class DbIdTest : public testing::Test {
  protected:

    DbIdTest() {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

    static DbId fromValidVariant(const QVariant& variant) {
        DbId actual(variant);
        EXPECT_TRUE(actual.isValid());
        EXPECT_NE(DbId(), actual);
        EXPECT_EQ(variant.toInt(), actual.toInt());
        return actual;
    }

    static DbId fromInvalidVariant(const QVariant& variant) {
        DbId actual(variant);
        EXPECT_FALSE(actual.isValid());
        EXPECT_EQ(DbId(), actual);
        return actual;
    }
};

TEST_F(DbIdTest, DefaultConstructor) {
    DbId actual;

    EXPECT_FALSE(actual.isValid());
}

TEST_F(DbIdTest, Invalid) {
    fromInvalidVariant(-1);
    fromInvalidVariant(-12);
    fromInvalidVariant(-123);
    fromInvalidVariant(-1234);
    fromInvalidVariant("-1234");
    fromInvalidVariant("invalid id");
}

TEST_F(DbIdTest, Valid) {
    fromValidVariant(0);
    fromValidVariant(1);
    fromValidVariant(12);
    fromValidVariant(123);
    fromValidVariant(1234);
    EXPECT_EQ(fromValidVariant(1234), fromValidVariant(" 1234  "));
}

}  // namespace
