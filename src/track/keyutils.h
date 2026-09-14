#pragma once

#include <QList>
#include <QMutex>
#include <QString>

#include "audio/types.h"
#include "control/controlproxy.h"
#include "proto/keys.pb.h"
#include "track/keys.h"
#include "util/color/colorpalette.h"
#include "util/math.h"
#include "util/types.h"

class KeyUtils {
  public:
    enum class KeyNotation {
        Invalid = 0,
        Custom = 1,
        OpenKey = 2,
        Lancelot = 3,
        Traditional = 4,
        OpenKeyAndTraditional = 5,
        LancelotAndTraditional = 6,
        ID3v2 = 7,
        NumKeyNotations = 8
    };

    enum class ScaleMode {
        Ionian = 0,     // standard major
        Aeolian = 1,    // natural minor
        Lydian = 2,     // major with raised 4th
        Mixolydian = 3, // major with lowered 7th
        Dorian = 4,     // minor with raised 6th
        Phrygian = 5,   // minor with lowered 2nd
        Locrian = 6,    // minor with lowered 2nd and 7th
        Unknown = 7
    };

    static QString keyDebugName(mixxx::track::io::key::ChromaticKey key);

    static inline bool keyIsMajor(mixxx::track::io::key::ChromaticKey key) {
        return key > mixxx::track::io::key::INVALID &&
                key < mixxx::track::io::key::C_MINOR;
    }

    /// Returns the tonic, 0-indexed.
    static inline int keyToTonic(mixxx::track::io::key::ChromaticKey key) {
        if (key == mixxx::track::io::key::INVALID) {
            return mixxx::track::io::key::INVALID;
        }
        return static_cast<int>(key) - (keyIsMajor(key) ? 1 : 13);
    }

    /// Takes a 0-indexed tonic and whether it is major/minor and produces a key.
    static inline mixxx::track::io::key::ChromaticKey tonicToKey(int tonic, bool major) {
        return static_cast<mixxx::track::io::key::ChromaticKey>(
            tonic + (major ? 1 : 13));
    }

    // Converts a minor key to its relative major. This will change the tonic.
    static inline mixxx::track::io::key::ChromaticKey minorToRelativeMajor(
            mixxx::track::io::key::ChromaticKey key) {
        return openKeyNumberToKey(keyToOpenKeyNumber(key), true);
    }

    // Key to 0-based Major scale value. Converts Minor keys to relative majors.
    // eg. C and Am will be 0.0
    static inline double keyToScalePitch(mixxx::track::io::key::ChromaticKey key) {
        return keyToNumericValue(minorToRelativeMajor(key)) - 1;
    }

    // Ensure pitch is in the [0-12) range
    static inline double normalizePitch(double pitch) {
        double normPitch = fmod(pitch, 12.0);
        if (normPitch < 0) {
            normPitch += 12.0;
        }
        return normPitch;
    }

    // Given pitch difference of 2 keys, returns their distance on the keywheel
    static inline int pitchDiffToKeywheelSteps(int pitchDiff) {
        // Open Key number also conveniently gives us the clockwise index in the Keywheel
        // Keys use 1-based indexing (0 is INVALID)
        const int CWSteps = keyToOpenKeyNumber(keyFromNumericValue(pitchDiff + 1)) - 1;
        // it's a wheel, so check if counter-clockwise direction has fewer steps
        return std::min(CWSteps, 12 - CWSteps);
    }

    // returns the shortest pitch difference up or down, handling octaves
    static inline double shortestPitchDiff(double pitch1, double pitch2) {
        const double normPitchDiff = normalizePitch(pitch1 - pitch2);
        return std::min(normPitchDiff, 12.0 - normPitchDiff);
    }

    static QString keyToString(mixxx::track::io::key::ChromaticKey key,
            KeyNotation notation = KeyNotation::Custom);

    static QString formatGlobalKey(
            const Keys& keys,
            KeyNotation notation = KeyNotation::Custom);

    static mixxx::track::io::key::ChromaticKey keyFromNumericValue(double value);
    static mixxx::track::io::key::ChromaticKey keyFromNumericValue(int value);
    static KeyNotation keyNotationFromNumericValue(double value);
    static KeyNotation keyNotationFromString(const QString& notationName);

    static double keyToNumericValue(mixxx::track::io::key::ChromaticKey key);

    static QColor keyToColor(mixxx::track::io::key::ChromaticKey key, const ColorPalette& palette);

    static QPair<mixxx::track::io::key::ChromaticKey, double> scaleKeyOctaves(
        mixxx::track::io::key::ChromaticKey key, double scale);

    static mixxx::track::io::key::ChromaticKey scaleKeySteps(
        mixxx::track::io::key::ChromaticKey key, int steps);

    static inline double stepsToOctaveChange(int steps) {
        return static_cast<double>(steps) / 12.0;
    }

    static int shortestStepsToKey(mixxx::track::io::key::ChromaticKey key,
                                  mixxx::track::io::key::ChromaticKey target_key);

    static int shortestStepsToCompatibleKey(mixxx::track::io::key::ChromaticKey key,
                                            mixxx::track::io::key::ChromaticKey target_key);

    /// Returns a list of keys that are harmonically compatible with key using
    /// the Circle of Fifths (including the key itself).
    static QList<mixxx::track::io::key::ChromaticKey> getCompatibleKeys(
        mixxx::track::io::key::ChromaticKey key);

    static mixxx::track::io::key::ChromaticKey guessKeyFromText(const QString& text);

    static mixxx::track::io::key::ChromaticKey calculateGlobalKey(
            const KeyChangeList& key_changes,
            SINT totalFrames,
            mixxx::audio::SampleRate sampleRate);

    static void setNotation(
        const QMap<mixxx::track::io::key::ChromaticKey, QString>& notation);

    /// Returns pow(2, octaveChange)
    static inline double octaveChangeToPowerOf2(const double& octaveChange) {
        // Some libraries (e.g. SoundTouch) calculate pow(2, octaveChange)
        // using this identity:
        // x^y = e^(y*ln(x))
        // Presumably because exp() is faster than pow() on some machines.
        // TODO(rryan) benchmark on Intel / AMD / ARM processors and pick accordingly.
        //static const lg2 = log(2); // 0.69314718056f
        //return exp(lg2 * octaveChange);
        return pow(2.0, octaveChange);
    }

    static inline double semitoneChangeToPowerOf2(const double& semitones) {
        return octaveChangeToPowerOf2(semitones / 12);
    }

    static inline double powerOf2ToOctaveChange(const double& power_of_2) {
        return log2(power_of_2);
    }

    static inline double powerOf2ToSemitoneChange(const double& power_of_2) {
        return powerOf2ToOctaveChange(power_of_2) * 12;
    }

    static mixxx::track::io::key::ChromaticKey openKeyNumberToKey(int openKeyNumber, bool major);

    static int keyToOpenKeyNumber(mixxx::track::io::key::ChromaticKey key);

    static int keyToCircleOfFifthsOrder(mixxx::track::io::key::ChromaticKey key,
                                        KeyNotation notation);

    static double trackSyncPitchDifference(double key1, double bpm1, double key2, double bpm2);

    static double trackSimilarity(double key1, double bpm1, double key2, double bpm2);

    static QVariant keyFromKeyTextAndIdFields(
            const QVariant& keyTextField, const QVariant& keyIdField);
    static QString keyFromKeyTextAndIdValues(const QString& keyText,
            const mixxx::track::io::key::ChromaticKey& key);

  private:
    static QMutex s_notationMutex;
    static QMap<mixxx::track::io::key::ChromaticKey, QString> s_notation;
    static QMap<QString, mixxx::track::io::key::ChromaticKey> s_reverseNotation;
};
