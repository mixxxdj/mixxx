#include <gtest/gtest.h>

#include <QtDebug>

#include "util/db/dbid.h"

namespace {

class DbIdTest : public testing::Test {
  protected:
    static DbId fromValidVariant(const QVariant& variant) {
        DbId actual(variant);
        return fromValid(actual);
    }

    static DbId fromValid(const DbId& actual) {
        EXPECT_TRUE(actual.isValid());
        EXPECT_NE(DbId(), actual);

        EXPECT_EQ(actual.toVariant().typeId(), QMetaType::Int);
        EXPECT_NE(actual.toVariant().toInt(), -1);
        EXPECT_FALSE(actual.toVariant().isNull());

        EXPECT_EQ(actual.toVariantOrNull().typeId(), QMetaType::Int);
        EXPECT_NE(actual.toVariantOrNull().toInt(), -1);
        EXPECT_FALSE(actual.toVariantOrNull().isNull());
        return actual;
    }

    static DbId fromInvalidVariant(const QVariant& variant) {
        DbId actual(variant);
        return fromInvalid(actual);
    }

    static DbId fromInvalid(const DbId& actual) {
        EXPECT_FALSE(actual.isValid());
        EXPECT_EQ(DbId(), actual);

        EXPECT_EQ(actual.toVariant().typeId(), QMetaType::Int);
        EXPECT_EQ(actual.toVariant().toInt(), -1);
        EXPECT_FALSE(actual.toVariant().isNull());

        EXPECT_EQ(actual.toVariantOrNull().typeId(), QMetaType::Int);
        EXPECT_NE(actual.toVariantOrNull().toInt(), -1);
        EXPECT_TRUE(actual.toVariantOrNull().isNull());
        return actual;
    }
};

TEST_F(DbIdTest, DefaultConstructor) {
    DbId actual;

    EXPECT_FALSE(actual.isValid());
}

TEST_F(DbIdTest, Invalid) {
    fromInvalid(DbId());
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
