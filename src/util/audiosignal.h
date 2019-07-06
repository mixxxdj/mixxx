#pragma once

#include "util/assert.h"
#include "util/types.h"


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
    enum class ChannelLayout {
        Unknown,
        Mono,     // 1 channel
        DualMono, // 2 channels with identical signals
        Stereo,   // 2 independent channels left/right
        // ...
    };

    class ChannelCount {
      private:
        static constexpr SINT kValueDefault = 0;

      public:
        static constexpr SINT kValueMin     = 1;   // lower bound (inclusive)
        static constexpr SINT kValueMax     = 255; // upper bound (inclusive, 8-bit unsigned integer)

        static constexpr ChannelCount min() { return ChannelCount(kValueMin); }
        static constexpr ChannelCount max() { return ChannelCount(kValueMax); }

        static ChannelCount from(ChannelLayout channelLayout) {
            switch (channelLayout) {
            case ChannelLayout::Unknown:
                return ChannelCount();
            case ChannelLayout::Mono:
                return ChannelCount(1);
            case ChannelLayout::DualMono:
                return ChannelCount(1);
            case ChannelLayout::Stereo:
                return ChannelCount(2);
            }
            DEBUG_ASSERT(!"unreachable code");
            return ChannelCount();
        }

        explicit constexpr ChannelCount(SINT value = kValueDefault)
            : m_value(value) {
        }
        explicit ChannelCount(ChannelLayout channelLayout)
            : m_value(from(channelLayout).m_value) {
        }

        bool valid() const {
            return (kValueMin <= m_value) && (m_value <= kValueMax);
        }

        /*implicit*/ constexpr operator SINT() const {
            return m_value;
        }

      private:
        SINT m_value;
    };

    // Defines the ordering of how samples from multiple channels are
    // stored in contiguous buffers:
    //    - Planar: Channel by channel
    //    - Interleaved: Frame by frame
    // The samples from all channels that are coincident in time are
    // called a "frame" (or more specific "sample frame").
    //
    // Example: 10 stereo samples from left (L) and right (R) channel
    // Planar layout:      LLLLLLLLLLRRRRRRRRRR
    // Interleaved layout: LRLRLRLRLRLRLRLRLRLR
    enum class SampleLayout {
        Planar,
        Interleaved
    };

    class SampleRate {
      private:
        static constexpr SINT kValueDefault = 0;

      public:
        static constexpr SINT kValueMin     = 8000;   // lower bound (inclusive, = minimum MP3 sample rate)
        static constexpr SINT kValueMax     = 192000; // upper bound (inclusive)

        static constexpr SampleRate min() { return SampleRate(kValueMin); }
        static constexpr SampleRate max() { return SampleRate(kValueMax); }

        static constexpr const char* unit() { return "Hz"; }

        explicit constexpr SampleRate(SINT value = kValueDefault)
            : m_value(value) {
        }

        bool valid() const {
            return (kValueMin <= m_value) && (m_value <= kValueMax);
        }

        /*implicit*/ constexpr operator SINT() const {
            return m_value;
        }

      private:
        SINT m_value;
    };

    explicit AudioSignal(
            SampleLayout sampleLayout)
        : m_sampleLayout(sampleLayout) {
    }

    explicit AudioSignal(
            SampleLayout sampleLayout,
            ChannelCount channelCount,
            SampleRate sampleRate)
        : m_sampleLayout(sampleLayout) {
        setChannelCount(channelCount);
        setSampleRate(sampleRate);
    }

    virtual ~AudioSignal() = default;

    // Returns the ordering of samples in contiguous buffers.
    SampleLayout sampleLayout() const {
        return m_sampleLayout;
    }

    // Returns the number of channels.
    ChannelCount channelCount() const {
        return m_channelCount;
    }

    // Returns the sample rate in Hz. The sample rate is defined as the
    // number of samples per second for each channel. Please note that this
    // does not equal the total number of samples per second in the stream!
    //
    // NOTE(uklotzde): I consciously avoided the term "sample rate", because
    // that sounds like "number of samples per second" which is wrong for
    // signals with more than a single channel and might be misleading!
    SampleRate sampleRate() const {
        return m_sampleRate;
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
        DEBUG_ASSERT(channelCount().valid());
        DEBUG_ASSERT(0 == (samples % channelCount()));
        return samples / channelCount();
    }

    // Conversion: #frames / frame offset -> #samples / sample offset
    template<typename T>
    inline T frames2samples(T frames) const {
        DEBUG_ASSERT(channelCount().valid());
        return frames * channelCount();
    }

protected:
    bool setChannelCount(ChannelCount channelCount);
    bool setChannelCount(SINT channelCount) {
        return setChannelCount(ChannelCount(channelCount));
    }
    bool setSampleRate(SampleRate sampleRate);
    bool setSampleRate(SINT sampleRate) {
        return setSampleRate(SampleRate(sampleRate));
    }

private:
    ChannelCount m_channelCount;
    SampleLayout m_sampleLayout;
    SampleRate m_sampleRate;
};

QDebug operator<<(QDebug dbg, AudioSignal::SampleLayout arg);

QDebug operator<<(QDebug dbg, const AudioSignal& arg);

}
