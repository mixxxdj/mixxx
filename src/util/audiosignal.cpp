#include "util/audiosignal.h"

#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("AudioSignal");

} // anonymous namespace

bool AudioSignal::setChannelCount(audio::ChannelCount channelCount) {
    if (channelCount < audio::ChannelCount()) {
        kLogger.warning()
                << "Invalid channel count"
                << channelCount;
        return false; // abort
    } else {
        m_signalInfo.setChannelCount(channelCount);
        return true;
    }
}

bool AudioSignal::setSampleRate(audio::SampleRate sampleRate) {
    if (sampleRate < audio::SampleRate()) {
        kLogger.warning()
                << "Invalid sample rate"
                << sampleRate;
        return false; // abort
    } else {
        m_signalInfo.setSampleRate(sampleRate);
        return true;
    }
}

bool AudioSignal::verifyReadable() const {
    bool result = true;
    if (!channelCount().isValid()) {
        kLogger.warning()
                << "Invalid number of channels:"
                << channelCount()
                << "is out of range ["
                << audio::ChannelCount::min()
                << ","
                << audio::ChannelCount::max()
                << "]";
        result = false;
    }
    if (!sampleRate().isValid()) {
        kLogger.warning()
                << "Invalid sample rate:"
                << sampleRate()
                << "is out of range ["
                << audio::SampleRate::min()
                << ","
                << audio::SampleRate::max()
                << "]";
        result = false;
    }
    return result;
}

QDebug operator<<(QDebug dbg, const AudioSignal& arg) {
    return dbg
            << "AudioSignal{"
            << arg.getSignalInfo()
            << "}";
}

} // namespace mixxx
