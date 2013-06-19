#include <gtest/gtest.h>
#include <QDebug>

#include "track/keyutils.h"
#include "proto/keys.pb.h"

namespace {

class KeyUtilsTest : public testing::Test {
};

TEST_F(KeyUtilsTest, OpenKeyNotation) {
    EXPECT_EQ(mixxx::track::io::key::C_MAJOR,
              KeyUtils::guessKeyFromText("1D"));
    EXPECT_EQ(mixxx::track::io::key::C_SHARP_MINOR,
              KeyUtils::guessKeyFromText("5M"));

    // Lower-case.
    EXPECT_EQ(mixxx::track::io::key::B_MAJOR,
              KeyUtils::guessKeyFromText("6d"));
    EXPECT_EQ(mixxx::track::io::key::B_MINOR,
              KeyUtils::guessKeyFromText("3m"));
}

TEST_F(KeyUtilsTest, LancelotNotation) {
    EXPECT_EQ(mixxx::track::io::key::C_MAJOR,
              KeyUtils::guessKeyFromText("8B"));
    EXPECT_EQ(mixxx::track::io::key::C_SHARP_MINOR,
              KeyUtils::guessKeyFromText("12A"));

    // Lower-case.
    EXPECT_EQ(mixxx::track::io::key::B_MAJOR,
              KeyUtils::guessKeyFromText("1b"));
    EXPECT_EQ(mixxx::track::io::key::B_MINOR,
              KeyUtils::guessKeyFromText("10a"));
}

TEST_F(KeyUtilsTest, KeyNameNotation) {
    // Invalid letter.
    EXPECT_EQ(mixxx::track::io::key::INVALID,
              KeyUtils::guessKeyFromText("H"));

    // Major is upper-case, minor is lower-case.
    EXPECT_EQ(mixxx::track::io::key::C_MAJOR,
              KeyUtils::guessKeyFromText("C"));
    EXPECT_EQ(mixxx::track::io::key::C_MINOR,
              KeyUtils::guessKeyFromText("c"));

    // Sharps
    EXPECT_EQ(mixxx::track::io::key::D_FLAT_MAJOR,
              KeyUtils::guessKeyFromText("C#"));

    // Flats
    EXPECT_EQ(mixxx::track::io::key::D_FLAT_MAJOR,
              KeyUtils::guessKeyFromText("Db"));

    // Mixed sharps and flats.
    EXPECT_EQ(mixxx::track::io::key::C_MINOR,
              KeyUtils::guessKeyFromText("c#bb#"));

    // No matter what ending in m is minor.
    EXPECT_EQ(mixxx::track::io::key::C_MINOR,
              KeyUtils::guessKeyFromText("CM"));
    EXPECT_EQ(mixxx::track::io::key::C_MINOR,
              KeyUtils::guessKeyFromText("cm"));
    EXPECT_EQ(mixxx::track::io::key::C_SHARP_MINOR,
              KeyUtils::guessKeyFromText("C#m"));
    EXPECT_EQ(mixxx::track::io::key::C_SHARP_MINOR,
              KeyUtils::guessKeyFromText("Dbm"));
    EXPECT_EQ(mixxx::track::io::key::C_MINOR,
              KeyUtils::guessKeyFromText("C#bb#m"));

    // Test going across the edges.
    EXPECT_EQ(mixxx::track::io::key::B_MAJOR,
              KeyUtils::guessKeyFromText("Cb"));
    EXPECT_EQ(mixxx::track::io::key::C_MAJOR,
              KeyUtils::guessKeyFromText("B#"));
    EXPECT_EQ(mixxx::track::io::key::B_MINOR,
              KeyUtils::guessKeyFromText("cb"));
    EXPECT_EQ(mixxx::track::io::key::C_MINOR,
              KeyUtils::guessKeyFromText("b#"));
}




}  // namespace
