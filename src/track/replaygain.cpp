#include "track/replaygain.h"

#include "util/math.h"

namespace Mixxx {

// TODO(uklotzde): Replace 'const' with 'constexpr' and remove
// initialization after switching to Visual Studio 2015.

/*static*/ const double ReplayGain::kRatioUndefined = 0.0;
/*static*/ const double ReplayGain::kRatioMin = 0.0; // lower bound (exclusive)
/*static*/ const double ReplayGain::kRatio0dB = 1.0;

/*static*/ const CSAMPLE ReplayGain::kPeakUndefined = -CSAMPLE_PEAK;
/*static*/ const CSAMPLE ReplayGain::kPeakMin = CSAMPLE_ZERO; // lower bound (inclusive)
/*static*/ const CSAMPLE ReplayGain::kPeakClip = CSAMPLE_PEAK; // upper bound (inclusive) represents digital full scale without clipping;

namespace {

const QString kGainUnit("dB");
const QString kGainSuffix(" " + kGainUnit);

QString stripLeadingSign(const QString& trimmed, char sign) {
    const int signIndex = trimmed.indexOf(sign);
    if (0 == signIndex) {
        return trimmed.mid(signIndex + 1).trimmed();
    } else {
        return trimmed;
    }
}

QString normalizeNumberString(const QString& number, bool* pValid) {
    if (pValid) {
        *pValid = false;
    }
    const QString trimmed(number.trimmed());
    QString normalized(stripLeadingSign(trimmed, '+'));
    if (normalized == trimmed) {
        // no leading '+' sign found
        if (pValid) {
            *pValid = true;
        }
        return normalized;
    } else {
        // stripped leading '+' sign -> no more leading signs '+'/'-' allowed
        if ((normalized == stripLeadingSign(normalized, '+')) &&
            (normalized == stripLeadingSign(normalized, '-'))) {
            if (pValid) {
                *pValid = true;
            }
            return normalized;
        }
    }
    // normalization failed
    return number;
}

} // anonymous namespace

double ReplayGain::ratioFromString(QString dbGain, bool* pValid) {
    if (pValid) {
        *pValid = false;
    }
    bool isValid = false;
    QString normalizedGain(normalizeNumberString(dbGain, &isValid));
    if (!isValid) {
        return kRatioUndefined;
    }
    const int unitIndex = normalizedGain.lastIndexOf(kGainUnit, -1, Qt::CaseInsensitive);
    if ((0 <= unitIndex) && ((normalizedGain.length() - 2) == unitIndex)) {
        // strip trailing unit suffix
        normalizedGain = normalizedGain.left(unitIndex).trimmed();
    }
    if (normalizedGain.isEmpty()) {
        return kRatioUndefined;
    }
    isValid = false;
    const double replayGainDb = normalizedGain.toDouble(&isValid);
    if (isValid) {
        const double ratio = db2ratio(replayGainDb);
        DEBUG_ASSERT(kRatioUndefined != ratio);
        if (isValidRatio(ratio)) {
            if (pValid) {
                *pValid = true;
            }
            return ratio;
        } else {
            qDebug() << "ReplayGain: Invalid gain value:" << dbGain << " -> "<< ratio;
        }
    } else {
        qDebug() << "ReplayGain: Failed to parse gain:" << dbGain;
    }
    return kRatioUndefined;
}

QString ReplayGain::ratioToString(double ratio) {
    if (isValidRatio(ratio)) {
        return QString::number(ratio2db(ratio)) + kGainSuffix;
    } else {
        return QString();
    }
}

double ReplayGain::normalizeRatio(double ratio) {
    if (isValidRatio(ratio)) {
        const double normalizedRatio = ratioFromString(ratioToString(ratio));
        // NOTE(uklotzde): Subsequently formatting and parsing the
        // normalized value should not alter it anymore!
        DEBUG_ASSERT(normalizedRatio == ratioFromString(ratioToString(normalizedRatio)));
        return normalizedRatio;
    } else {
        return kRatioUndefined;
    }
}

CSAMPLE ReplayGain::peakFromString(QString strPeak, bool* pValid) {
    if (pValid) {
        *pValid = false;
    }
    bool isValid = false;
    QString normalizedPeak(normalizeNumberString(strPeak, &isValid));
    if (!isValid || normalizedPeak.isEmpty()) {
        return kPeakUndefined;
    }
    isValid = false;
    const CSAMPLE peak = normalizedPeak.toDouble(&isValid);
    if (isValid) {
        if (isValidPeak(peak)) {
            if (pValid) {
                *pValid = true;
            }
            return peak;
        } else {
            qDebug() << "ReplayGain: Invalid peak value:" << strPeak << " -> "<< peak;
        }
    } else {
        qDebug() << "ReplayGain: Failed to parse peak:" << strPeak;
    }
    return kPeakUndefined;
}

QString ReplayGain::peakToString(CSAMPLE peak) {
    if (isValidPeak(peak)) {
        return QString::number(peak);
    } else {
        return QString();
    }
}

CSAMPLE ReplayGain::normalizePeak(CSAMPLE peak) {
    if (isValidPeak(peak)) {
        const CSAMPLE normalizedPeak = peakFromString(peakToString(peak));
        // NOTE(uklotzde): Subsequently formatting and parsing the
        // normalized value should not alter it anymore!
        DEBUG_ASSERT(normalizedPeak == peakFromString(peakToString(normalizedPeak)));
        return normalizedPeak;
    } else {
        return kPeakUndefined;
    }
}

} //namespace Mixxx
