#include <gtest/gtest.h>

#include "util/quuid.h"

class UuidTest : public testing::Test {
};

TEST_F(UuidTest, uuidToNullableStringWithoutBracesNullTest) {
    EXPECT_STREQ(
            qPrintable(QString{}),
            qPrintable(uuidToNullableStringWithoutBraces(QUuid{})));
}

TEST_F(UuidTest, uuidToNullableStringWithoutBracesValidTest) {
    const auto validUuid = QUuid::createUuid();
    ASSERT_FALSE(validUuid.isNull());
    EXPECT_STREQ(
            qPrintable(validUuid.toString(QUuid::WithoutBraces)),
            qPrintable(uuidToNullableStringWithoutBraces(validUuid)));
}
