#include <QtDebug>

#include "util/audiosignal.h"

namespace mixxx {

bool AudioSignal::verifyReadable() const {
    bool result = true;
    if (!hasValidChannelCount()) {
        qWarning() << "Invalid number of channels:"
                << getChannelCount()
                << "is out of range ["
                << kChannelCountMin
                << ","
                << kChannelCountMax
                << "]";
        result = false;
    }
    if (!hasValidSamplingRate()) {
        qWarning() << "Invalid sampling rate [Hz]:"
                << getSamplingRate()
                << "is out of range ["
                << kSamplingRateMin
                << ","
                << kSamplingRateMax
                << "]";
        result = false;
    }
    return result;
}

} // namespace mixxx
