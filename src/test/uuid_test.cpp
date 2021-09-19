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
    const QString validUuidString = QStringLiteral("93a20486-90fd-4ea5-a869-ef669cacb0b2");
    EXPECT_STREQ(
            qPrintable(validUuidString),
            qPrintable(uuidToNullableStringWithoutBraces(QUuid(validUuidString))));
}
