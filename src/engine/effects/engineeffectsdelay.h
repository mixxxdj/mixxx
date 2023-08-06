#pragma once

#include "engine/engine.h"
#include "engine/engineobject.h"
#include "util/assert.h"
#include "util/types.h"

namespace {
static constexpr int kMaxDelayFrames =
        mixxx::audio::SampleRate::kValueMax - 1;
static constexpr int kDelayBufferSize =
        mixxx::audio::SampleRate::kValueMax * mixxx::kEngineChannelOutputCount;
} // anonymous namespace

/// The effect can produce the output signal with a specific delay caused
/// by the inner effect processing. Based on that the signal on the input
/// of the effect does not overlap with the signal on the effect output.
/// For using the effect in the Dry/Wet or Dry+Wet mode the dry signal
/// and wet signal do not overlap and the effect latency is recognizable.
///
/// The EngineEffectsDelay offers a solution to this situation by delaying
/// the input dry signal by a specific amount of delay. The signal delaying
/// handles the class method EngineEffectsDelay::process. The method
/// can be used for the one specific effect same as the effect chain if
/// the set delay is the delay of the whole effect chain (sum of the delay
/// of all effects in the effect chain).
///
/// After delaying the non-delayed signal, both signals (delayed
/// and non-delayed) can be mixed and used together.
class EngineEffectsDelay final : public EngineObject {
    Q_OBJECT
  public:
    EngineEffectsDelay();

    ~EngineEffectsDelay() override;

    /// Called from the audio thread

    /// The method sets the number of frames of which will
    /// the EngineEffectsDelay::process method delays the input signal.
    /// By default, the EngineEffectsDelay::process method works with
    /// a zero delay. When is the delay set, the EngineEffectsDelay::process
    /// method works with this set delay value until the value is changed.
    void setDelayFrames(SINT delayFrames) {
        if (delayFrames < 0) {
            DEBUG_ASSERT_UNREACHABLE(false);
            delayFrames = 0;
        }
        if (delayFrames > kMaxDelayFrames) {
            DEBUG_ASSERT_UNREACHABLE(false);
            delayFrames = kMaxDelayFrames;
        }

        // Delay is reported from the effect chain by a number of frames
        // to aware problems with a number of channels. The inner
        // EngineEffectsDelay structure works with delay samples, so the value
        // is recalculated for the EngineEffectsDelay usage.
        m_currentDelaySamples = delayFrames * mixxx::kEngineChannelOutputCount;
    }

    /// The method delays the input buffer by the set number of samples
    /// and returns the result in the output buffer. The input buffer
    /// is not changed. For zero delay the input buffer is copied into
    /// the output buffer. For non-zero delay, the output signal is returned
    /// with the delay.
    ///
    /// If the number of delay samples hasn't changed, between two
    /// EngineEffectsDelay::process method calls, the delayed signal buffers
    /// directly follow each other as an input buffer, however, with a delay.
    ///
    /// If the number of delayed samples has changed between two
    /// EngineEffectsDelay::process method calls, the new delay value is set
    /// as actual and the output buffer is filled using cross-fading
    /// of the presumed output buffer for the previous delay value
    /// and of the output buffer created using the new delay value.
    void process(CSAMPLE* pInOut, const std::size_t bufferSize) override;

  private:
    SINT m_currentDelaySamples;
    SINT m_prevDelaySamples;
    SINT m_delayBufferWritePos;
    CSAMPLE* m_pDelayBuffer;
};
