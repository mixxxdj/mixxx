#ifndef MIXXX_REPLAYGAIN_H
#define MIXXX_REPLAYGAIN_H

#include "util/types.h"

namespace Mixxx {

// DTO for replay gain. Must not be subclassed (no virtual destructor)!
class ReplayGain {
public:
    static const double kRatioUndefined;
    static const double kRatioMin; // lower bound (exclusive)
    static const double kRatio0dB;

    ReplayGain()
        : m_ratio(kRatioUndefined)
        , m_peak(CSAMPLE_PEAK) {
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

    // Parse and format replay gain metadata according to the ReplayGain
    // 1.0/2.0 specification.
    // http://wiki.hydrogenaud.io/index.php?title=ReplayGain_1.0_specification
    // http://wiki.hydrogenaud.io/index.php?title=ReplayGain_2.0_specification
    static double parseGain2Ratio(QString dBGain, bool* pValid = 0);
    static QString formatRatio2Gain(double ratio);

    // After normalization formatting and parsing the ratio repeatedly will
    // always lead to the same value. This is required to reliably store the
    // dB gain as a string in track metadata.
    static double normalizeRatio(double ratio);

    // The peak amplitude of the track or signal.
    CSAMPLE getPeak() const {
        return m_ratio;
    }
    void setPeak(CSAMPLE peak) {
        m_peak = peak;
    }
    void resetPeak() {
        m_peak = CSAMPLE_PEAK;
    }

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

#endif // MIXXX_REPLAYGAIN_H
