#ifndef MIXXX_REPLAYGAIN_H
#define MIXXX_REPLAYGAIN_H

#include "util/types.h"

namespace Mixxx {

// DTO for storing replay gain information. This class cannot be subclassed,
// because the destructor is not virtual!
//
// Parsing & Formatting
// --------------------
// This class includes functions for formatting and parsing replay gain
// metadata according to the ReplayGain 1.0/2.0 specification:
// http://wiki.hydrogenaud.io/index.php?title=ReplayGain_1.0_specification
// http://wiki.hydrogenaud.io/index.php?title=ReplayGain_2.0_specification
//
// Normalization
// -------------
// Formatting a floating point value as a string and parsing it later
// might cause rounding errors. In order to avoid "jittering" caused
// by subsequently formatting and parsing a floating point value the
// ratio and peak values need to be normalized before writing them
// as a string into file tags.
class ReplayGain {
public:
    static const double kRatioUndefined;
    static const double kRatioMin; // lower bound (exclusive)
    static const double kRatio0dB;

    static const CSAMPLE kPeakUndefined;
    static const CSAMPLE kPeakMin; // lower bound (inclusive)
    static const CSAMPLE kPeakClip; // upper bound (inclusive) without clipping

    ReplayGain()
        : ReplayGain(kRatioUndefined, kPeakUndefined) {
    }
    ReplayGain(double ratio, CSAMPLE peak)
        : m_ratio(ratio)
        , m_peak(peak) {
    }

    static bool isValidRatio(double ratio) {
        return kRatioMin < ratio;
    }
    bool hasRatio() const {
        return isValidRatio(m_ratio);
    }
    double getRatio() const {
        return m_ratio;
    }
    void setRatio(double ratio) {
        m_ratio = ratio;
    }
    void resetRatio() {
        m_ratio = kRatioUndefined;
    }

    static double parseGain2Ratio(QString dBGain, bool* pValid = 0);
    static QString formatRatio2Gain(double ratio);

    static double normalizeRatio(double ratio);

    // The peak amplitude of the track or signal.
    static bool isValidPeak(CSAMPLE peak) {
        return kPeakMin <= peak;
    }
    bool hasPeak() const {
        return isValidPeak(m_peak);
    }
    CSAMPLE getPeak() const {
        return m_peak;
    }
    void setPeak(CSAMPLE peak) {
        m_peak = peak;
    }
    void resetPeak() {
        m_peak = CSAMPLE_PEAK;
    }

    static CSAMPLE parsePeak(QString strPeak, bool* pValid = 0);
    static QString formatPeak(CSAMPLE peak);

    static CSAMPLE normalizePeak(CSAMPLE peak);

private:
    double m_ratio;
    CSAMPLE m_peak;
};

inline
bool operator==(const ReplayGain& lhs, const ReplayGain& rhs) {
    return (lhs.getRatio() == rhs.getRatio()) && (lhs.getPeak() == rhs.getPeak());
}

inline
bool operator!=(const ReplayGain& lhs, const ReplayGain& rhs) {
    return !(lhs == rhs);
}

}

Q_DECLARE_METATYPE(Mixxx::ReplayGain)

#endif // MIXXX_REPLAYGAIN_H
