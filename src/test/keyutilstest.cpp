#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QDebug>

#include "track/keyutils.h"
#include "proto/keys.pb.h"

using ::testing::ElementsAre;

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

    // whitespace is ok
    EXPECT_EQ(mixxx::track::io::key::B_MINOR,
              KeyUtils::guessKeyFromText(" 3m\t\t"));
    // but other stuff is not
    EXPECT_EQ(mixxx::track::io::key::INVALID,
              KeyUtils::guessKeyFromText(" 33m\t\t"));
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

    // whitespace is ok
    EXPECT_EQ(mixxx::track::io::key::B_MINOR,
              KeyUtils::guessKeyFromText("\t10a  "));
    // but other stuff is not
    EXPECT_EQ(mixxx::track::io::key::INVALID,
              KeyUtils::guessKeyFromText("\t10aa  "));
}

TEST_F(KeyUtilsTest, KeyNameNotation) {
    // Invalid letter
    // (actually valid in traditional german notation where B is H and Bb is B -
    //  everyone confused?)
    EXPECT_EQ(mixxx::track::io::key::INVALID,
              KeyUtils::guessKeyFromText("H"));

    // whitespace is ok
    EXPECT_EQ(mixxx::track::io::key::C_MAJOR,
              KeyUtils::guessKeyFromText(" \tC   \t  "));
    // but other crap is not
    EXPECT_EQ(mixxx::track::io::key::INVALID,
              KeyUtils::guessKeyFromText(" c\tC   c\t  "));

    // Major is upper-case, minor is lower-case.
    EXPECT_EQ(mixxx::track::io::key::C_MAJOR,
              KeyUtils::guessKeyFromText("C"));
    EXPECT_EQ(mixxx::track::io::key::C_MINOR,
              KeyUtils::guessKeyFromText("c"));

    // ... or explicitly written out
    EXPECT_EQ(mixxx::track::io::key::C_MAJOR,
              KeyUtils::guessKeyFromText("cmaj"));
    EXPECT_EQ(mixxx::track::io::key::C_MINOR,
              KeyUtils::guessKeyFromText("Cmin"));
    EXPECT_EQ(mixxx::track::io::key::C_MAJOR,
              KeyUtils::guessKeyFromText("cmajor"));
    EXPECT_EQ(mixxx::track::io::key::C_MINOR,
              KeyUtils::guessKeyFromText("Cminor"));

    // Sharps
    EXPECT_EQ(mixxx::track::io::key::D_FLAT_MAJOR,
              KeyUtils::guessKeyFromText("C#"));
    EXPECT_EQ(mixxx::track::io::key::D_FLAT_MAJOR,
              KeyUtils::guessKeyFromText(QString::fromUtf8("C♯")));

    // Flats
    EXPECT_EQ(mixxx::track::io::key::D_FLAT_MAJOR,
              KeyUtils::guessKeyFromText("Db"));
    EXPECT_EQ(mixxx::track::io::key::D_FLAT_MAJOR,
              KeyUtils::guessKeyFromText(QString::fromUtf8("D♭")));

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

    // ... as is min
    EXPECT_EQ(mixxx::track::io::key::C_MINOR,
              KeyUtils::guessKeyFromText("CMiN"));
    EXPECT_EQ(mixxx::track::io::key::C_MINOR,
              KeyUtils::guessKeyFromText("cmIn"));
    EXPECT_EQ(mixxx::track::io::key::C_SHARP_MINOR,
              KeyUtils::guessKeyFromText("C#mIN"));
    EXPECT_EQ(mixxx::track::io::key::C_SHARP_MINOR,
              KeyUtils::guessKeyFromText("Dbmin"));
    EXPECT_EQ(mixxx::track::io::key::C_MINOR,
              KeyUtils::guessKeyFromText("C#bb#miN"));

    // but maj is major
    EXPECT_EQ(mixxx::track::io::key::C_MAJOR,
              KeyUtils::guessKeyFromText("CMaJ"));
    EXPECT_EQ(mixxx::track::io::key::C_MAJOR,
              KeyUtils::guessKeyFromText("cmAj"));
    EXPECT_EQ(mixxx::track::io::key::D_FLAT_MAJOR,
              KeyUtils::guessKeyFromText("C#mAJ"));
    EXPECT_EQ(mixxx::track::io::key::D_FLAT_MAJOR,
              KeyUtils::guessKeyFromText("Dbmaj"));
    EXPECT_EQ(mixxx::track::io::key::C_MAJOR,
              KeyUtils::guessKeyFromText("C#bb#maJ"));

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

mixxx::track::io::key::ChromaticKey incrementKey(
    mixxx::track::io::key::ChromaticKey key, int steps=1) {
    return static_cast<mixxx::track::io::key::ChromaticKey>(
        static_cast<int>(key) + steps);
}

TEST_F(KeyUtilsTest, ShortestStepsToKey_EqualKeyZeroSteps) {
    mixxx::track::io::key::ChromaticKey key = mixxx::track::io::key::INVALID;
    while (true) {
        EXPECT_EQ(0, KeyUtils::shortestStepsToKey(key, key));
        if (key == mixxx::track::io::key::B_MINOR) {
            break;
        }
        key = incrementKey(key);
    }
}

TEST_F(KeyUtilsTest, ShortestStepsToKey_SameTonicZeroSteps) {
    mixxx::track::io::key::ChromaticKey minor = mixxx::track::io::key::C_MINOR;
    mixxx::track::io::key::ChromaticKey major = mixxx::track::io::key::C_MAJOR;

    while (true) {
        EXPECT_EQ(0, KeyUtils::shortestStepsToKey(minor, major));
        if (minor == mixxx::track::io::key::B_MINOR) {
            break;
        }
        major = incrementKey(major);
        minor = incrementKey(minor);
    }
}

TEST_F(KeyUtilsTest, ShortestStepsToKey) {
    mixxx::track::io::key::ChromaticKey start_key_minor =
            mixxx::track::io::key::C_MINOR;
    mixxx::track::io::key::ChromaticKey start_key_major =
            mixxx::track::io::key::C_MAJOR;

    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 12; ++j) {
            mixxx::track::io::key::ChromaticKey minor_key = KeyUtils::scaleKeySteps(start_key_minor, j);
            mixxx::track::io::key::ChromaticKey major_key = KeyUtils::scaleKeySteps(start_key_major, j);
            // When we are 6 steps away, 6 and -6 are equidistant.
            if (j == 6) {
                EXPECT_EQ(6, abs(KeyUtils::shortestStepsToKey(start_key_minor, minor_key)));
                EXPECT_EQ(6, abs(KeyUtils::shortestStepsToKey(start_key_minor, major_key)));
                EXPECT_EQ(6, abs(KeyUtils::shortestStepsToKey(start_key_major, minor_key)));
                EXPECT_EQ(6, abs(KeyUtils::shortestStepsToKey(start_key_major, major_key)));
            } else {
                int expected = (j < 6) ? j : j - 12;
                EXPECT_EQ(expected, KeyUtils::shortestStepsToKey(start_key_minor, minor_key));
                EXPECT_EQ(expected, KeyUtils::shortestStepsToKey(start_key_minor, major_key));
                EXPECT_EQ(expected, KeyUtils::shortestStepsToKey(start_key_major, minor_key));
                EXPECT_EQ(expected, KeyUtils::shortestStepsToKey(start_key_major, major_key));
            }
        }
        start_key_minor = KeyUtils::scaleKeySteps(start_key_minor, 1);
        start_key_major = KeyUtils::scaleKeySteps(start_key_major, 1);
    }
}

TEST_F(KeyUtilsTest, GetCompatibleKeys) {
    // The relative major, the perfect 4th and the perfect 5th are all
    // compatible. This is easily checked with the Circle of Fifths.

    // Test keys on the boundary between 1 and 12 to check that wrap-around
    // works.
    mixxx::track::io::key::ChromaticKey key =
            mixxx::track::io::key::A_MINOR;
    QList<mixxx::track::io::key::ChromaticKey> compatible =
            KeyUtils::getCompatibleKeys(key);
    qSort(compatible);
    EXPECT_THAT(compatible, ElementsAre(
        mixxx::track::io::key::C_MAJOR,
        mixxx::track::io::key::D_MINOR,
        mixxx::track::io::key::E_MINOR,
        mixxx::track::io::key::A_MINOR));

    key = mixxx::track::io::key::F_MAJOR;
    compatible = KeyUtils::getCompatibleKeys(key);
    qSort(compatible);
    EXPECT_THAT(compatible, ElementsAre(
        mixxx::track::io::key::C_MAJOR,
        mixxx::track::io::key::F_MAJOR,
        mixxx::track::io::key::B_FLAT_MAJOR,
        mixxx::track::io::key::D_MINOR));
}

}  // namespace
