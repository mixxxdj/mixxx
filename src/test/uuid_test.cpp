#include <gtest/gtest.h>

#include "util/quuid.h"

class UuidTest : public testing::Test {
};

TEST_F(UuidTest, uuidToNullableStringWithoutBracesNullTest) {
    EXPECT_EQ(QString(),
            uuidToNullableStringWithoutBraces(QUuid()));
}

TEST_F(UuidTest, uuidToNullableStringWithoutBracesValidTest) {
    EXPECT_EQ(QString("93a20486-90fd-4ea5-a869-ef669cacb0b2"),
            uuidToNullableStringWithoutBraces(QUuid("93a20486-90fd-4ea5-a869-ef669cacb0b2")));
}
