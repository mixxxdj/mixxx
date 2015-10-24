#ifndef MIXXX_AUDIOSIGNAL_H
#define MIXXX_AUDIOSIGNAL_H

#include "util/assert.h"
#include "util/types.h"

namespace Mixxx {

// Common properties of audio signals in Mixxx.
//
// An audio signal describes a stream of samples for multiple channels.
// If there is more than one channel, the samples of each channel will
// be interleaved. The samples from all channels that are coincident
// in time are called a "frame". Consequently audio signals are streamed
// frame by frame. With the knowledge about the number of channels sample
// and frame offsets in the stream can be converted vice versa.
//
// Internally each sample is represented by a floating-point value.
//
// The properties of an audio signal are immutable and must be constant
// over time. Therefore all functions for modifying individual properties
// are declared as "protected" and only available from derived classes.
class AudioSignal {
public:
    static const SINT kChannelCountZero    = 0;
    static const SINT kChannelCountMono    = 1;
    static const SINT kChannelCountStereo  = 2;
    static const SINT kChannelCountDefault = kChannelCountZero;

    static bool isValidChannelCount(SINT channelCount) {
        return kChannelCountZero < channelCount;
    }

    static const SINT kSamplingRateZero    = 0;
    static const SINT kSamplingRateCD      = 44100;
    static const SINT kSamplingRate48kHz   = 48000;
    static const SINT kSamplingRate96kHz   = 96000;
    static const SINT kSamplingRateDefault = kSamplingRateZero;

    static bool isValidSamplingRate(SINT samplingRate) {
        return kSamplingRateZero < samplingRate;
    }

    AudioSignal()
        : m_channelCount(kChannelCountDefault),
          m_samplingRate(kSamplingRateDefault) {
    }
    AudioSignal(SINT channelCount, SINT samplingRate)
        : m_channelCount(channelCount),
          m_samplingRate(samplingRate) {
        DEBUG_ASSERT(kChannelCountZero <= m_channelCount);
        DEBUG_ASSERT(kSamplingRateZero <= m_samplingRate);
    }
    virtual ~AudioSignal() {}

    // Returns the number of channels.
    SINT getChannelCount() const {
        return m_channelCount;
    }
    bool hasChannelCount() const {
        return isValidChannelCount(getChannelCount());
    }

    // Returns the sampling rate in Hz. The sampling rate is defined as the
    // number of samples per second for each channel. Please not that this
    // does not equal the total number of samples per second in the stream!
    //
    // NOTE(uklotzde): I consciously avoided the term "sample rate", because
    // that sounds like "number of samples per second" which is wrong for
    // signals with more than a single channel and might be misleading!
    SINT getSamplingRate() const {
        return m_samplingRate;
    }
    bool hasSamplingRate() const {
        return isValidSamplingRate(getSamplingRate());
    }

    // Check for valid properties. Subclasses may override this function
    // to add more constraints. Derived functions should always call the
    // implementation of the super class and concatenate the result with
    // && (logical and).
    virtual bool isValid() const {
        return hasChannelCount() && hasSamplingRate();
    }

    // Conversion: #samples / sample offset -> #frames / frame offset
    template<typename T>
    inline T samples2frames(T samples) const {
        DEBUG_ASSERT(hasChannelCount());
        DEBUG_ASSERT(0 == (samples % getChannelCount()));
        return samples / getChannelCount();
    }

    // Conversion: #frames / frame offset -> #samples / sample offset
    template<typename T>
    inline T frames2samples(T frames) const {
        DEBUG_ASSERT(hasChannelCount());
        return frames * getChannelCount();
    }

protected:
    void setChannelCount(SINT channelCount) {
        DEBUG_ASSERT(isValidChannelCount(channelCount));
        m_channelCount = channelCount;
    }
    void resetChannelCount() {
        m_channelCount = kChannelCountDefault;
    }

    void setSamplingRate(SINT samplingRate) {
        DEBUG_ASSERT(isValidSamplingRate(samplingRate));
        m_samplingRate = samplingRate;
    }
    void resetSamplingRate() {
        m_samplingRate = kSamplingRateDefault;
    }

private:
    SINT m_channelCount;
    SINT m_samplingRate;
};

}

#endif // MIXXX_AUDIOSIGNAL_H
