#ifndef KEYUTILS_H
#define KEYUTILS_H

#include <QString>
#include <cmath>

#include "proto/keys.pb.h"
#include "track/keys.h"

class KeyUtils {
  public:
    enum KeyNotation {
        // The default notation (set with setNotation).
        DEFAULT = 0,
        OPEN_KEY = 1,
        LANCELOT = 2,
        TRADITIONAL = 3,
    };

    static const char* keyDebugName(mixxx::track::io::key::ChromaticKey key);

    static inline bool keyIsMajor(mixxx::track::io::key::ChromaticKey key) {
        return key > mixxx::track::io::key::INVALID &&
                key < mixxx::track::io::key::C_MINOR;
    }

    static QString keyToString(mixxx::track::io::key::ChromaticKey key,
                               KeyNotation notation=DEFAULT);

    static mixxx::track::io::key::ChromaticKey keyFromNumericValue(double value);

    static double keyToNumericValue(mixxx::track::io::key::ChromaticKey key);

    static mixxx::track::io::key::ChromaticKey scaleKeyOctaves(
        mixxx::track::io::key::ChromaticKey key, double scale);

    static mixxx::track::io::key::ChromaticKey scaleKeySteps(
        mixxx::track::io::key::ChromaticKey key, int steps);

    static mixxx::track::io::key::ChromaticKey keyToRelativeMajorOrMinor(
        mixxx::track::io::key::ChromaticKey key);

    static mixxx::track::io::key::ChromaticKey guessKeyFromText(const QString& text);

    static mixxx::track::io::key::ChromaticKey calculateGlobalKey(
        const KeyChangeList& key_changes, int iTotalSamples);

    static void setNotation(
        const QMap<mixxx::track::io::key::ChromaticKey, QString>& notation);

    // Returns pow(2, octaveChange)
    static inline double octaveChangeToPowerOf2(const double& octaveChange) {
        // Some libraries (e.g. SoundTouch) calculate pow(2, octaveChange)
        // using this identity:
        // x^y = e^(y*ln(x))
        // Presumably because exp() is faster than pow() on some machines.
        // TODO(rryan) benchmark on Intel / AMD / ARM processors and pick accordingly.
        //static const lg2 = log(2); // 0.69314718056f
        //return exp(lg2 * octaveChange);
        return pow(2, octaveChange);
    }

    static inline double powerOf2ToOctaveChange(const double& power_of_2) {
        // log2 is in the C99 standard, MSVC only supports C90.
#ifdef _MSC_VER
        static const double lg2 = log(2.0);
        return log(power_of_2) / lg2;
#else
        return log2(power_of_2);
#endif
    }

  private:
    static QMutex s_notationMutex;
    static QMap<mixxx::track::io::key::ChromaticKey, QString> s_notation;
    static QMap<QString, mixxx::track::io::key::ChromaticKey> s_reverseNotation;
};

#endif /* KEYUTILS_H */
