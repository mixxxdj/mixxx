#include <gtest/gtest.h>

#include <QtDebug>

#include "util/db/dbid.h"

class DbIdTest : public testing::Test {};

// An arbitrary valid value
constexpr DbId::value_type kSomeValidValue = 123;
static_assert(DbId::kMinValue != kSomeValidValue);

// An arbitrary invalid value
constexpr DbId::value_type kSomeInvalidValue = -123;
static_assert(DbId::kInvalidValue != kSomeInvalidValue);

constexpr DbId kSomeInvalidId = DbId{DbId::NoAssert{}, kSomeInvalidValue};

TEST_F(DbIdTest, fromInvalidValue) {
    EXPECT_FALSE(DbId{}.isValid());
    EXPECT_EQ(DbId{}, DbId{DbId::kInvalidValue});
}

TEST_F(DbIdTest, fromValidValue) {
    EXPECT_TRUE(DbId{DbId::kMinValue}.isValid());
    EXPECT_TRUE(DbId{kSomeValidValue}.isValid());
}

TEST_F(DbIdTest, toValue) {
    EXPECT_EQ(DbId::kInvalidValue, DbId{}.valueMaybeInvalid());
    EXPECT_EQ(DbId::kMinValue, DbId{DbId::kMinValue}.value());
    EXPECT_EQ(kSomeValidValue, DbId{kSomeValidValue}.value());
    EXPECT_EQ(kSomeInvalidValue, kSomeInvalidId.valueMaybeInvalid());
}

TEST_F(DbIdTest, fromInvalidVariant) {
    EXPECT_EQ(DbId{}, DbId{QVariant{}});
}

TEST_F(DbIdTest, fromVariantWithValue) {
    EXPECT_EQ(DbId{}, DbId{QVariant{DbId::kInvalidValue}});
    EXPECT_EQ(DbId{DbId::kMinValue}, DbId{QVariant{DbId::kMinValue}});
    EXPECT_EQ(DbId{kSomeValidValue}, DbId{QVariant{kSomeValidValue}});
    EXPECT_EQ(DbId{}, DbId{QVariant{kSomeInvalidValue}});
}

TEST_F(DbIdTest, fromVariantWithString) {
    EXPECT_EQ(DbId{}, DbId{QVariant{QString::number(DbId::kInvalidValue)}});
    EXPECT_EQ(DbId{DbId::kMinValue}, DbId{QVariant{QString::number(DbId::kMinValue)}});
    EXPECT_EQ(DbId{kSomeValidValue}, DbId{QVariant{QString::number(kSomeValidValue)}});
    EXPECT_EQ(DbId{}, DbId{QVariant{QString::number(kSomeInvalidValue)}});
    EXPECT_EQ(DbId{}, DbId{QVariant{QStringLiteral("one")}});
}

TEST_F(DbIdTest, toVariant) {
    EXPECT_EQ(QVariant{}, DbId{}.toVariant());
    EXPECT_EQ(QVariant{DbId::kMinValue}, DbId{DbId::kMinValue}.toVariant());
    EXPECT_EQ(QVariant{kSomeValidValue}, DbId{kSomeValidValue}.toVariant());
    EXPECT_EQ(QVariant{}, kSomeInvalidId.toVariant());
}

TEST_F(DbIdTest, toString) {
    EXPECT_EQ(QString{}, DbId{}.toString());
    EXPECT_EQ(QString::number(DbId::kMinValue), DbId{DbId::kMinValue}.toString());
    EXPECT_EQ(QString::number(kSomeValidValue), DbId{kSomeValidValue}.toString());
    EXPECT_EQ(QString{}, kSomeInvalidId.toString());
}
