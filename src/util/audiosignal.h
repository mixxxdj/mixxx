#pragma once

#include "audio/signalinfo.h"
#include "util/assert.h"

namespace mixxx {

// Common properties of audio signals in Mixxx.
//
// An audio signal describes a stream of samples for multiple channels.
// Internally each sample is represented by a floating-point value.
//
// The properties of an audio signal are immutable and must be constant
// over time. Therefore all functions for modifying individual properties
// are declared as "protected" and are only available from derived classes.
class AudioSignal {
  public:
    explicit AudioSignal(
            audio::SampleLayout sampleLayout)
            : m_signalInfo(std::make_optional(sampleLayout)) {
    }

    explicit AudioSignal(
            audio::SignalInfo signalInfo)
            : m_signalInfo(signalInfo) {
        DEBUG_ASSERT(signalInfo.isValid());
    }

    virtual ~AudioSignal() = default;

    const audio::SignalInfo& getSignalInfo() const {
        DEBUG_ASSERT(m_signalInfo.isValid());
        return m_signalInfo;
    }

    audio::ChannelCount channelCount() const {
        return getSignalInfo().getChannelCount();
    }

    audio::SampleRate sampleRate() const {
        return getSignalInfo().getSampleRate();
    }

    // Verifies various properties to ensure that the audio data is
    // actually readable. Warning messages are logged for properties
    // with invalid values for diagnostic purposes.
    //
    // Subclasses may override this function for checking additional
    // properties in derived classes. Derived functions should always
    // call the implementation of the super class first:
    //
    // bool DerivedClass::verifyReadable() const {
    //     bool result = BaseClass::validate();
    //     if (my property is invalid) {
    //         qWarning() << ...warning message...
    //         result = false;
    //     }
    //     return result;
    // }
    virtual bool verifyReadable() const;

    // Conversion: #samples / sample offset -> #frames / frame offset
    template<typename T>
    inline T samples2frames(T samples) const {
        return getSignalInfo().samples2frames(samples);
    }

    // Conversion: #frames / frame offset -> #samples / sample offset
    template<typename T>
    inline T frames2samples(T frames) const {
        return getSignalInfo().frames2samples(frames);
    }

  protected:
    bool setChannelCount(audio::ChannelCount channelCount);
    bool setChannelCount(SINT channelCount) {
        return setChannelCount(audio::ChannelCount(channelCount));
    }
    bool setSampleRate(audio::SampleRate sampleRate);
    bool setSampleRate(SINT sampleRate) {
        return setSampleRate(audio::SampleRate(sampleRate));
    }

  private:
    audio::SignalInfo m_signalInfo;
};

QDebug operator<<(QDebug dbg, const AudioSignal& arg);

} // namespace mixxx
