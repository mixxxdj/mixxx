#include <gtest/gtest.h>

#include "util/quuid.h"

class UuidTest : public testing::Test {
};

TEST_F(UuidTest, uuidToStringWithoutBracesNullTest) {
    EXPECT_EQ(QString("00000000-0000-0000-0000-000000000000"),
            uuidToStringWithoutBraces(QUuid()));
}

TEST_F(UuidTest, uuidToNullableStringWithoutBracesNullTest) {
    EXPECT_EQ(QString(),
            uuidToNullableStringWithoutBraces(QUuid()));
}

TEST_F(UuidTest, uuidToStringWithoutBracesValidTest) {
    EXPECT_EQ(QString("93a20486-90fd-4ea5-a869-ef669cacb0b2"),
            uuidToStringWithoutBraces(QUuid("93a20486-90fd-4ea5-a869-ef669cacb0b2")));
}

TEST_F(UuidTest, uuidToNullableStringWithoutBracesValidTest) {
    EXPECT_EQ(QString("93a20486-90fd-4ea5-a869-ef669cacb0b2"),
            uuidToNullableStringWithoutBraces(QUuid("93a20486-90fd-4ea5-a869-ef669cacb0b2")));
}
