#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QDebug>
#include <QMap>
#include <QString>

#include "proto/keys.pb.h"
#include "track/keyutils.h"

using ::testing::UnorderedElementsAre;

class KeyUtilsTest : public testing::Test {
};

TEST_F(KeyUtilsTest, OpenKeyNotation) {
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

    // Allow lower-case to be more flexible when parsing search queries
    EXPECT_EQ(mixxx::track::io::key::C_MAJOR,
            KeyUtils::guessKeyFromText("8b"));
    EXPECT_EQ(mixxx::track::io::key::C_SHARP_MINOR,
            KeyUtils::guessKeyFromText("12a"));

    // whitespace is ok
    EXPECT_EQ(mixxx::track::io::key::B_MINOR,
            KeyUtils::guessKeyFromText("\t10A  "));
    // but other stuff is not
    EXPECT_EQ(mixxx::track::io::key::INVALID,
            KeyUtils::guessKeyFromText("\t10AA  "));
}

TEST_F(KeyUtilsTest, KeyNameNotation) {
    // Invalid letter
    // (actually valid in traditional German notation where B is H and Bb is B -
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

    // Rapid Evolution test cases
    EXPECT_EQ(mixxx::track::io::key::A_MINOR,
            KeyUtils::guessKeyFromText("Am"));
    EXPECT_EQ(mixxx::track::io::key::A_MINOR,
            KeyUtils::guessKeyFromText("08A"));
    EXPECT_EQ(mixxx::track::io::key::B_FLAT_MINOR,
            KeyUtils::guessKeyFromText("A#m"));
    EXPECT_EQ(mixxx::track::io::key::B_FLAT_MINOR,
            KeyUtils::guessKeyFromText("Bbm"));
    EXPECT_EQ(mixxx::track::io::key::A_MAJOR,
            KeyUtils::guessKeyFromText("11B"));
    EXPECT_EQ(mixxx::track::io::key::A_FLAT_MAJOR, // ionian
            KeyUtils::guessKeyFromText("G#+50"));
    EXPECT_EQ(mixxx::track::io::key::A_FLAT_MAJOR, // ionian
            KeyUtils::guessKeyFromText("Ab +50"));
    EXPECT_EQ(mixxx::track::io::key::A_FLAT_MAJOR, // ionian
            KeyUtils::guessKeyFromText("Ab +50cents"));
    EXPECT_EQ(mixxx::track::io::key::A_FLAT_MAJOR, // ionian
            KeyUtils::guessKeyFromText("04B +50cents"));
    EXPECT_EQ(mixxx::track::io::key::A_FLAT_MAJOR, // ionian
            KeyUtils::guessKeyFromText("G#-50"));
    EXPECT_EQ(mixxx::track::io::key::A_FLAT_MAJOR, // ionian
            KeyUtils::guessKeyFromText("Ab -50"));
    EXPECT_EQ(mixxx::track::io::key::A_FLAT_MAJOR, // ionian
            KeyUtils::guessKeyFromText("Ab -50cents"));
    EXPECT_EQ(mixxx::track::io::key::A_FLAT_MAJOR, // ionian
            KeyUtils::guessKeyFromText("04B -50cents"));
    // Mixxx does not allow this but Rapid Evolution
    // EXPECT_EQ(mixxx::track::io::key::A_FLAT_MAJOR, // ionian
    //      KeyUtils::guessKeyFromText("    4b    -50   cents    "));
    // EXPECT_EQ(mixxx::track::io::key::A_FLAT_MAJOR, // ionian
    //          KeyUtils::guessKeyFromText("    g  #    -    50   cents    "));
    // EXPECT_EQ(mixxx::track::io::key::A_FLAT_MAJOR, // ionian
    //          KeyUtils::guessKeyFromText("    g  #    +    50   cents    "));
    EXPECT_EQ(mixxx::track::io::key::INVALID, // ionian
            KeyUtils::guessKeyFromText(" "));
    EXPECT_EQ(mixxx::track::io::key::INVALID, // ionian
            KeyUtils::guessKeyFromText(""));
    EXPECT_EQ(mixxx::track::io::key::INVALID, // ionian
            KeyUtils::guessKeyFromText("xyz"));
}

TEST_F(KeyUtilsTest, ScaleModeNotation) {
    EXPECT_EQ(mixxx::track::io::key::C_MAJOR,
            KeyUtils::guessKeyFromText("C ionian"));
    EXPECT_EQ(mixxx::track::io::key::A_MINOR,
            KeyUtils::guessKeyFromText("A aeolian"));
    EXPECT_EQ(mixxx::track::io::key::C_MAJOR,
            KeyUtils::guessKeyFromText("F lydian"));
    EXPECT_EQ(mixxx::track::io::key::C_MAJOR,
            KeyUtils::guessKeyFromText("G mixolydian"));
    EXPECT_EQ(mixxx::track::io::key::A_MINOR,
            KeyUtils::guessKeyFromText("D dorian"));
    EXPECT_EQ(mixxx::track::io::key::A_MINOR,
            KeyUtils::guessKeyFromText("E phrygian"));
    EXPECT_EQ(mixxx::track::io::key::A_MINOR,
            KeyUtils::guessKeyFromText("B locrian"));

    EXPECT_EQ(mixxx::track::io::key::F_SHARP_MINOR,
            KeyUtils::guessKeyFromText("11A"));
    EXPECT_EQ(mixxx::track::io::key::A_MAJOR,
            KeyUtils::guessKeyFromText("11B"));
    EXPECT_EQ(mixxx::track::io::key::A_MAJOR,
            KeyUtils::guessKeyFromText("11I"));
    EXPECT_EQ(mixxx::track::io::key::A_MAJOR,
            KeyUtils::guessKeyFromText("11L"));
    EXPECT_EQ(mixxx::track::io::key::A_MAJOR,
            KeyUtils::guessKeyFromText("11M"));
    EXPECT_EQ(mixxx::track::io::key::F_SHARP_MINOR,
            KeyUtils::guessKeyFromText("11D"));
    EXPECT_EQ(mixxx::track::io::key::F_SHARP_MINOR,
            KeyUtils::guessKeyFromText("11P"));
    EXPECT_EQ(mixxx::track::io::key::F_SHARP_MINOR,
            KeyUtils::guessKeyFromText("11C"));

    // Redundant Mode
    EXPECT_EQ(mixxx::track::io::key::INVALID,
            KeyUtils::guessKeyFromText("Cm ionian"));
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
    // The relative major/minor, the perfect 4th major/minor and the
    // perfect 5th major/minor are all compatible. This is easily
    // checked with the Circle of Fifths.

    // Test keys on the boundary between 1 and 12 to check that wrap-around
    // works.
    mixxx::track::io::key::ChromaticKey key =
            mixxx::track::io::key::A_MINOR;
    QList<mixxx::track::io::key::ChromaticKey> compatible =
            KeyUtils::getCompatibleKeys(key);
    EXPECT_THAT(compatible,
            UnorderedElementsAre(mixxx::track::io::key::C_MAJOR,
                    mixxx::track::io::key::F_MAJOR,
                    mixxx::track::io::key::G_MAJOR,
                    mixxx::track::io::key::D_MINOR,
                    mixxx::track::io::key::E_MINOR,
                    mixxx::track::io::key::A_MINOR));

    key = mixxx::track::io::key::F_MAJOR;
    compatible = KeyUtils::getCompatibleKeys(key);
    EXPECT_THAT(compatible,
            UnorderedElementsAre(mixxx::track::io::key::C_MAJOR,
                    mixxx::track::io::key::F_MAJOR,
                    mixxx::track::io::key::B_FLAT_MAJOR,
                    mixxx::track::io::key::D_MINOR,
                    mixxx::track::io::key::A_MINOR,
                    mixxx::track::io::key::G_MINOR));
}

// ---------------------------------------------------------------------------
// classifyAgainst() — harmonic key-highlighter classifier.
//
// The full input space is 25x25 = 625 (ChromaticKey, ChromaticKey) pairs, so
// these tests mix targeted cases with exhaustive sweeps over the valid keys.
// ---------------------------------------------------------------------------

namespace {

using mixxx::track::io::key::ChromaticKey;
using Class = KeyUtils::KeyHighlightClass;

// The first and last valid (non-INVALID) ChromaticKey values.
constexpr ChromaticKey kFirstKey = mixxx::track::io::key::C_MAJOR; // 1
constexpr ChromaticKey kLastKey = mixxx::track::io::key::B_MINOR;  // 24

} // anonymous namespace

TEST_F(KeyUtilsTest, ClassifyInvalidInputs) {
    // INVALID on either (or both) sides is always None.
    EXPECT_EQ(Class::None,
            KeyUtils::classifyAgainst(mixxx::track::io::key::INVALID,
                    mixxx::track::io::key::INVALID));
    for (ChromaticKey k = kFirstKey; k <= kLastKey; k = incrementKey(k)) {
        EXPECT_EQ(Class::None,
                KeyUtils::classifyAgainst(mixxx::track::io::key::INVALID, k))
                << "ref=" << static_cast<int>(k);
        EXPECT_EQ(Class::None,
                KeyUtils::classifyAgainst(k, mixxx::track::io::key::INVALID))
                << "track=" << static_cast<int>(k);
    }
}

TEST_F(KeyUtilsTest, ClassifySameKey) {
    // A key against itself is always the strongest match.
    for (ChromaticKey k = kFirstKey; k <= kLastKey; k = incrementKey(k)) {
        EXPECT_EQ(Class::GreenPerfect, KeyUtils::classifyAgainst(k, k))
                << "key=" << static_cast<int>(k);
    }
}

TEST_F(KeyUtilsTest, ClassifyRelativeMajorMinor) {
    // The relative major/minor sit on the same Circle-of-Fifths radial (same
    // OpenKey number) but in opposite modes, e.g. C major (8B) / A minor (8A).
    // Derive each major key's true relative minor from the OpenKey number and
    // verify both directions for all 12 radials.
    for (int i = 0; i < 12; ++i) {
        const auto major = static_cast<ChromaticKey>(
                static_cast<int>(mixxx::track::io::key::C_MAJOR) + i);
        const int openKeyNumber = KeyUtils::keyToOpenKeyNumber(major);
        const auto relativeMinor =
                KeyUtils::openKeyNumberToKey(openKeyNumber, /*major=*/false);
        ASSERT_FALSE(KeyUtils::keyIsMajor(relativeMinor));
        EXPECT_EQ(Class::GreenPerfect,
                KeyUtils::classifyAgainst(major, relativeMinor))
                << "major=" << static_cast<int>(major);
        EXPECT_EQ(Class::GreenPerfect,
                KeyUtils::classifyAgainst(relativeMinor, major))
                << "relativeMinor=" << static_cast<int>(relativeMinor);
    }
}

TEST_F(KeyUtilsTest, ClassifyCircleNeighbours) {
    // Non-relative members of the compatible set are GreenNeighbour (4th/5th).
    // Pin against A minor and F major (the keys GetCompatibleKeys verifies).
    const auto aMinor = mixxx::track::io::key::A_MINOR;
    EXPECT_EQ(Class::GreenNeighbour,
            KeyUtils::classifyAgainst(mixxx::track::io::key::F_MAJOR, aMinor));
    EXPECT_EQ(Class::GreenNeighbour,
            KeyUtils::classifyAgainst(mixxx::track::io::key::G_MAJOR, aMinor));
    EXPECT_EQ(Class::GreenNeighbour,
            KeyUtils::classifyAgainst(mixxx::track::io::key::D_MINOR, aMinor));
    EXPECT_EQ(Class::GreenNeighbour,
            KeyUtils::classifyAgainst(mixxx::track::io::key::E_MINOR, aMinor));
    // C major is the *relative* major of A minor, so it is GreenPerfect, not
    // GreenNeighbour.
    EXPECT_EQ(Class::GreenPerfect,
            KeyUtils::classifyAgainst(mixxx::track::io::key::C_MAJOR, aMinor));

    const auto fMajor = mixxx::track::io::key::F_MAJOR;
    EXPECT_EQ(Class::GreenNeighbour,
            KeyUtils::classifyAgainst(mixxx::track::io::key::C_MAJOR, fMajor));
    EXPECT_EQ(Class::GreenNeighbour,
            KeyUtils::classifyAgainst(mixxx::track::io::key::B_FLAT_MAJOR, fMajor));
    EXPECT_EQ(Class::GreenNeighbour,
            KeyUtils::classifyAgainst(mixxx::track::io::key::A_MINOR, fMajor));
    EXPECT_EQ(Class::GreenNeighbour,
            KeyUtils::classifyAgainst(mixxx::track::io::key::G_MINOR, fMajor));
    // D minor is the relative minor of F major -> GreenPerfect.
    EXPECT_EQ(Class::GreenPerfect,
            KeyUtils::classifyAgainst(mixxx::track::io::key::D_MINOR, fMajor));
}

TEST_F(KeyUtilsTest, ClassifyYellowSemitoneShift) {
    // A track is Yellow if it is not compatible itself, but +1 or -1 semitone
    // lands in the compatible set.
    const auto ref = mixxx::track::io::key::A_MINOR;
    const QList<ChromaticKey> compatible = KeyUtils::getCompatibleKeys(ref);

    // Find concrete +1 and -1 reachable examples by construction and assert the
    // classifier agrees, including the 12<->1 / 24<->13 wrap-around.
    bool sawPlusOne = false;
    bool sawMinusOne = false;
    for (ChromaticKey track = kFirstKey; track <= kLastKey;
            track = incrementKey(track)) {
        if (compatible.contains(track)) {
            continue; // those are green, not yellow
        }
        const bool plus = compatible.contains(KeyUtils::scaleKeySteps(track, 1));
        const bool minus = compatible.contains(KeyUtils::scaleKeySteps(track, -1));
        if (plus || minus) {
            EXPECT_EQ(Class::Yellow, KeyUtils::classifyAgainst(track, ref))
                    << "track=" << static_cast<int>(track);
            sawPlusOne = sawPlusOne || plus;
            sawMinusOne = sawMinusOne || minus;
        }
    }
    EXPECT_TRUE(sawPlusOne) << "expected at least one +1-semitone Yellow case";
    EXPECT_TRUE(sawMinusOne) << "expected at least one -1-semitone Yellow case";

    // Explicit wrap-around spot check: B major (id 12) +1 semitone wraps to
    // C major (id 1) within the same mode via scaleKeySteps.
    EXPECT_EQ(mixxx::track::io::key::C_MAJOR,
            KeyUtils::scaleKeySteps(mixxx::track::io::key::B_MAJOR, 1));
}

TEST_F(KeyUtilsTest, ClassifyRed) {
    // Keys that are neither compatible nor +/-1-semitone reachable are Red.
    const auto ref = mixxx::track::io::key::A_MINOR;
    const QList<ChromaticKey> compatible = KeyUtils::getCompatibleKeys(ref);
    int redCount = 0;
    for (ChromaticKey track = kFirstKey; track <= kLastKey;
            track = incrementKey(track)) {
        const bool green = compatible.contains(track);
        const bool yellow = !green &&
                (compatible.contains(KeyUtils::scaleKeySteps(track, 1)) ||
                        compatible.contains(KeyUtils::scaleKeySteps(track, -1)));
        if (!green && !yellow) {
            EXPECT_EQ(Class::Red, KeyUtils::classifyAgainst(track, ref))
                    << "track=" << static_cast<int>(track);
            ++redCount;
        }
    }
    EXPECT_GT(redCount, 0) << "expected at least one Red case for A minor";
}

TEST_F(KeyUtilsTest, ClassifyExhaustiveConsistency) {
    // Sweep all 24x24 valid pairs and assert invariants that must hold
    // regardless of the exact thresholds. This catches off-by-one and ordering
    // regressions across the whole input space, not just hand-picked points.
    for (ChromaticKey ref = kFirstKey; ref <= kLastKey; ref = incrementKey(ref)) {
        const QList<ChromaticKey> compatible = KeyUtils::getCompatibleKeys(ref);
        for (ChromaticKey track = kFirstKey; track <= kLastKey;
                track = incrementKey(track)) {
            const Class c = KeyUtils::classifyAgainst(track, ref);

            // Never None for valid inputs.
            EXPECT_NE(Class::None, c)
                    << "track=" << static_cast<int>(track)
                    << " ref=" << static_cast<int>(ref);

            // Identity is always GreenPerfect.
            if (track == ref) {
                EXPECT_EQ(Class::GreenPerfect, c);
            }

            const bool isGreen =
                    c == Class::GreenPerfect || c == Class::GreenNeighbour;
            const bool inCompatible = compatible.contains(track);

            // Green <=> membership in the compatible set (cross-check against
            // the existing helper).
            EXPECT_EQ(isGreen, inCompatible)
                    << "track=" << static_cast<int>(track)
                    << " ref=" << static_cast<int>(ref);

            // Yellow <=> not compatible AND +/-1 semitone is compatible
            // (mutual exclusivity with green is implied).
            const bool semitoneReachable =
                    compatible.contains(KeyUtils::scaleKeySteps(track, 1)) ||
                    compatible.contains(KeyUtils::scaleKeySteps(track, -1));
            if (c == Class::Yellow) {
                EXPECT_FALSE(inCompatible);
                EXPECT_TRUE(semitoneReachable);
            }

            // Red <=> neither compatible nor semitone-reachable.
            if (c == Class::Red) {
                EXPECT_FALSE(inCompatible);
                EXPECT_FALSE(semitoneReachable);
            }
        }
    }
}

TEST_F(KeyUtilsTest, ClassifyIsNotationIndependent) {
    // Classification compares ChromaticKey values, never display strings, so
    // setting a custom display notation must not change the result.
    const auto track = mixxx::track::io::key::E_MINOR;
    const auto ref = mixxx::track::io::key::A_MINOR;
    const Class before = KeyUtils::classifyAgainst(track, ref);

    // Install a bogus custom notation that renames every key, then re-classify.
    QMap<ChromaticKey, QString> bogus;
    for (ChromaticKey k = kFirstKey; k <= kLastKey; k = incrementKey(k)) {
        bogus.insert(k, QStringLiteral("x%1").arg(static_cast<int>(k)));
    }
    KeyUtils::setNotation(bogus);
    const Class after = KeyUtils::classifyAgainst(track, ref);

    EXPECT_EQ(before, after);
    EXPECT_EQ(Class::GreenNeighbour, after);
}
