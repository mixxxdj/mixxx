#include "util/replaygain.h"

#include "util/math.h"

namespace Mixxx {

/*static*/ const double ReplayGain::kRatioUndefined = 0.0;
/*static*/ const double ReplayGain::kRatioMin = 0.0;
/*static*/ const double ReplayGain::kRatio0dB = 1.0;

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

double ReplayGain::parseGain2Ratio(QString dbGain, bool* pValid) {
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

QString ReplayGain::formatRatio2Gain(double ratio) {
    if (isValidRatio(ratio)) {
        return QString::number(ratio2db(ratio)) + kGainSuffix;
    } else {
        return QString();
    }
}

double ReplayGain::normalizeRatio(double ratio) {
    if (isValidRatio(ratio)) {
        const double normalizedRatio = parseGain2Ratio(formatRatio2Gain(ratio));
        // NOTE(uklotzde): Subsequently formatting and parsing the
        // normalized value should not alter it anymore!
        DEBUG_ASSERT(normalizedRatio == parseGain2Ratio(formatRatio2Gain(normalizedRatio)));
        return normalizedRatio;
    } else {
        return kRatioUndefined;
    }
}

} //namespace Mixxx
