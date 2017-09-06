#include "util/audiosignal.h"

#include "util/logger.h"


namespace mixxx {

namespace {

const Logger kLogger("AudioSignal");

} // anonymous namespace

bool AudioSignal::setChannelCount(ChannelCount channelCount) {
    if (channelCount < ChannelCount()) {
        kLogger.warning()
                << "Invalid channel count"
                << channelCount;
        return false; // abort
    } else {
        m_channelCount = channelCount;
        return true;
    }
}

bool AudioSignal::setSamplingRate(SamplingRate samplingRate) {
    if (samplingRate < SamplingRate()) {
        kLogger.warning()
                << "Invalid sampling rate"
                << samplingRate;
        return false; // abort
    } else {
        m_samplingRate = samplingRate;
        return true;
    }
}

bool AudioSignal::verifyReadable() const {
    bool result = true;
    if (!channelCount().valid()) {
        kLogger.warning()
                << "Invalid number of channels:"
                << channelCount()
                << "is out of range ["
                << ChannelCount::min()
                << ","
                << ChannelCount::max()
                << "]";
        result = false;
    }
    if (!samplingRate().valid()) {
        kLogger.warning()
                << "Invalid sampling rate [Hz]:"
                << samplingRate()
                << "is out of range ["
                << SamplingRate::min()
                << ","
                << SamplingRate::max()
                << "]";
        result = false;
    }
    return result;
}

QDebug operator<<(QDebug dbg, AudioSignal::SampleLayout arg) {
    switch (arg) {
    case AudioSignal::SampleLayout::Planar:
        return dbg << "Planar";
    case AudioSignal::SampleLayout::Interleaved:
        return dbg << "Interleaved";
    }
    DEBUG_ASSERT(!"unreachable code");
    return dbg;
}

QDebug operator<<(QDebug dbg, const AudioSignal& arg) {
    return dbg << "AudioSignal{"
            << "sampleLayout:" << arg.sampleLayout()
            << "channelCount:" << arg.channelCount()
            << "samplingRate:" << arg.samplingRate()
            << "}";
}

} // namespace mixxx
