#include <QtDebug>

#include "util/audiosignal.h"

namespace mixxx {

// Separate definitions of static class constants are only required
// for LLVM CLang. Otherwise the linker complains about undefined
// symbols!?
#if defined(__clang__)
const SINT AudioSignal::kChannelCountZero;
const SINT AudioSignal::kChannelCountDefault;
const SINT AudioSignal::kChannelCountMono;
const SINT AudioSignal::kChannelCountMin;
const SINT AudioSignal::kChannelCountStereo;
const SINT AudioSignal::kChannelCountMax;
const SINT AudioSignal::kSamplingRateZero;
const SINT AudioSignal::kSamplingRateDefault;
const SINT AudioSignal::kSamplingRateMin;
const SINT AudioSignal::kSamplingRate32kHz;
const SINT AudioSignal::kSamplingRateCD;
const SINT AudioSignal::kSamplingRate48kHz;
const SINT AudioSignal::kSamplingRate96kHz;
const SINT AudioSignal::kSamplingRate192kHz;
const SINT AudioSignal::kSamplingRateMax;
#endif

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
