#include "util/replaygain.h"

#include "util/math.h"

namespace Mixxx {

/*static*/ const double ReplayGain::kRatioUndefined = 0.0;
/*static*/ const double ReplayGain::kRatioMin = 0.0; // lower bound (inclusive)
/*static*/ const double ReplayGain::kRatio0dB = 1.0;

namespace {

const QString kGainUnit("dB");
const QString kGainSuffix(" " + kGainUnit);

} // anonymous namespace

double ReplayGain::parseGain2Ratio(QString dbGain, bool* pValid) {
    if (pValid) {
        *pValid = false;
    }
    QString trimmedGain(dbGain.trimmed());
    const int plusIndex = trimmedGain.indexOf('+');
    if (0 == plusIndex) {
        // strip leading "+"
        trimmedGain = trimmedGain.mid(plusIndex + 1).trimmed();
    }
    const int unitIndex = trimmedGain.lastIndexOf(kGainUnit, -1, Qt::CaseInsensitive);
    if ((0 <= unitIndex) && ((trimmedGain.length() - 2) == unitIndex)) {
        // strip trailing unit suffix
        trimmedGain = trimmedGain.left(unitIndex).trimmed();
    }
    if (trimmedGain.isEmpty()) {
        return kRatioUndefined;
    }
    bool isValid = false;
    const double replayGainDb = trimmedGain.toDouble(&isValid);
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
        return ratio;
    }
}

} //namespace Mixxx
