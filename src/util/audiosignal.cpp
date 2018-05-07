#include <QtDebug>

#include "util/audiosignal.h"

namespace mixxx {

/*static*/ constexpr SINT AudioSignal::kChannelCountZero;
/*static*/ constexpr SINT AudioSignal::kChannelCountDefault;
/*static*/ constexpr SINT AudioSignal::kChannelCountMono;
/*static*/ constexpr SINT AudioSignal::kChannelCountMin;
/*static*/ constexpr SINT AudioSignal::kChannelCountStereo;
/*static*/ constexpr SINT AudioSignal::kChannelCountMax;

/*static*/ constexpr SINT AudioSignal::kSamplingRateZero;
/*static*/ constexpr SINT AudioSignal::kSamplingRateDefault;
/*static*/ constexpr SINT AudioSignal::kSamplingRateMin;
/*static*/ constexpr SINT AudioSignal::kSamplingRate32kHz;
/*static*/ constexpr SINT AudioSignal::kSamplingRateCD;
/*static*/ constexpr SINT AudioSignal::kSamplingRate48kHz;
/*static*/ constexpr SINT AudioSignal::kSamplingRate96kHz;
/*static*/ constexpr SINT AudioSignal::kSamplingRate192kHz;
/*static*/ constexpr SINT AudioSignal::kSamplingRateMax;

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
