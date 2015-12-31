#include <gtest/gtest.h>

#include "track/tracknumbers.h"

#include <QtDebug>

namespace {

class TrackNumbersTest : public testing::Test {
  protected:

    TrackNumbersTest() {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

    void fromStringEmpty(const QString& inputValue) {
        const std::pair<TrackNumbers, TrackNumbers::ParseResult> actualResult(
                TrackNumbers::fromString(inputValue));

        qDebug() << "fromString():" << inputValue << "->" << actualResult.first.toString();

        EXPECT_EQ(TrackNumbers::ParseResult::EMPTY, actualResult.second);
        EXPECT_TRUE(actualResult.first.isValid());
        EXPECT_EQ(TrackNumbers(), actualResult.first);
    }

    TrackNumbers fromStringValid(const QString& inputValue, const TrackNumbers& expectedResult) {
        const std::pair<TrackNumbers, TrackNumbers::ParseResult> actualResult(
                TrackNumbers::fromString(inputValue));

        qDebug() << "fromString():" << inputValue << "->" << actualResult.first.toString();

        EXPECT_EQ(TrackNumbers::ParseResult::VALID, actualResult.second);
        EXPECT_TRUE(actualResult.first.isValid());
        EXPECT_EQ(expectedResult, actualResult.first);

        return actualResult.first;
    }

    TrackNumbers fromStringInvalid(const QString& inputValue, const TrackNumbers& expectedResult) {
        const std::pair<TrackNumbers, TrackNumbers::ParseResult> actualResult(
                TrackNumbers::fromString(inputValue));

        qDebug() << "fromString():" << inputValue << "->" << actualResult.first.toString();

        EXPECT_EQ(TrackNumbers::ParseResult::INVALID, actualResult.second);
        EXPECT_EQ(expectedResult, actualResult.first);

        return actualResult.first;
    }

    void toString(const TrackNumbers& inputValue, const QString& expectedResult) {
        const QString actualResult(inputValue.toString());
        qDebug() << "toString():" << expectedResult << "->" << actualResult;
        EXPECT_EQ(expectedResult, actualResult);

        // Re-parse the formatted string if the input is
        // a valid TrackNumbers instance
        if (inputValue.isValid()) {
            if (actualResult.isEmpty()) {
                fromStringEmpty(actualResult);
            } else {
                fromStringValid(actualResult, inputValue);
            }
        }
    }
};

TEST_F(TrackNumbersTest, FromStringEmpty) {
    fromStringEmpty("");
    fromStringEmpty(" \t \n   ");
}

TEST_F(TrackNumbersTest, fromStringValid) {
    fromStringValid("0", TrackNumbers(0));
    fromStringValid("0/0", TrackNumbers(0, 0));
    fromStringValid("1 / 0", TrackNumbers(1, 0));
    fromStringValid(" 1\t/\t2  ", TrackNumbers(1, 2));
    fromStringValid("12/7", TrackNumbers(12, 7));
}

TEST_F(TrackNumbersTest, fromStringInvalid) {
    fromStringInvalid("-2", TrackNumbers(-2));
    fromStringInvalid("1/ -1", TrackNumbers(1, -1));
    fromStringInvalid("-2 / 1", TrackNumbers(-2, 1));
    fromStringInvalid("-3/-4", TrackNumbers(-3, -4));
    fromStringInvalid("1/a", TrackNumbers(1, TrackNumbers::kValueUndefined));
    fromStringInvalid("a /1", TrackNumbers(TrackNumbers::kValueUndefined, 1));
}

TEST_F(TrackNumbersTest, ToString) {
    toString(TrackNumbers(), QString(""));
    toString(TrackNumbers(0), QString(""));
    toString(TrackNumbers(-3), QString(""));
    toString(TrackNumbers(1), QString("1"));
    toString(TrackNumbers(12), QString("12"));
    toString(TrackNumbers(1, 0), QString("1"));
    toString(TrackNumbers(12, 0), QString("12"));
    toString(TrackNumbers(0, 1), QString("0/1"));
    toString(TrackNumbers(0, 12), QString("00/12")); // with padding
    toString(TrackNumbers(7, 12), QString("07/12")); // with padding
    toString(TrackNumbers(-7, 12), QString("00/12")); // with padding
    toString(TrackNumbers(-7, 123), QString("000/123")); // with padding
    toString(TrackNumbers(7, -12), QString("7"));
    toString(TrackNumbers(12, 7), QString("12/7"));
    toString(TrackNumbers(12, 34), QString("12/34"));
}

} // anonymous namespace
