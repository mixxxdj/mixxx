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

bool AudioSignal::setSampleRate(SampleRate sampleRate) {
    if (sampleRate < SampleRate()) {
        kLogger.warning()
                << "Invalid sample rate"
                << sampleRate;
        return false; // abort
    } else {
        m_sampleRate = sampleRate;
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
    if (!sampleRate().valid()) {
        kLogger.warning()
                << "Invalid sample rate [Hz]:"
                << sampleRate()
                << "is out of range ["
                << SampleRate::min()
                << ","
                << SampleRate::max()
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
            << "sampleRate:" << arg.sampleRate()
            << "}";
}

} // namespace mixxx
