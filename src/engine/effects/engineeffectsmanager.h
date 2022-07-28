#pragma once

#include <QScopedPointer>

#include "engine/effects/groupfeaturestate.h"
#include "engine/effects/message.h"
#include "util/fifo.h"
#include "util/samplebuffer.h"
#include "util/types.h"

class EngineEffectChain;
class EngineEffect;

/// EngineEffectsManager is the entry point for processing effects in the audio
/// thread. It also passes EffectsRequests from EffectsMessenger down to the
/// EngineEffectChains and EngineEffects at the beginning of the audio thread cycle.
///
/// Signal flow diagram (disable word wrapping in your editor to read):
/// EngineChannel ---> EqualizerEffectChains --> channel faders & crossfader --> QuickEffectChains & StandardEffectChains --> mix channels into main mix --> main mix effect processing
///                                          |
///                                      PFL switch --> QuickEffectChains & StandardEffectChains --> mix channels into headphone mix --> headphone effect processing
class EngineEffectsManager final : public EffectsRequestHandler {
  public:
    EngineEffectsManager(EffectsResponsePipe* pResponsePipe);
    ~EngineEffectsManager();

    void onCallbackStart();

    /// Process the prefader EngineEffectChains on the pInOut buffer, modifying
    /// the contents of the input buffer.
    void processPreFaderInPlace(
            GroupHandle inputHandle,
            GroupHandle outputHandle,
            CSAMPLE* pInOut,
            const unsigned int numSamples,
            const unsigned int sampleRate);

    /// Process the postfader EngineEffectChains on the pInOut buffer, modifying
    /// the contents of the input buffer.
    void processPostFaderInPlace(
            GroupHandle inputHandle,
            GroupHandle outputHandle,
            CSAMPLE* pInOut,
            const unsigned int numSamples,
            const unsigned int sampleRate,
            const GroupFeatureState& groupFeatures,
            const CSAMPLE_GAIN oldGain = CSAMPLE_GAIN_ONE,
            const CSAMPLE_GAIN newGain = CSAMPLE_GAIN_ONE);

    /// Process the postfader EngineEffectChains, leaving the pIn buffer unmodified
    /// and mixing the output into the pOut buffer. Using EngineEffectsManager's
    /// temporary buffers for this avoids the need for ChannelMixer to allocate a
    /// buffer for every channel, which would potentially require allocation on the
    /// audio thread because ChannelMixer supports an arbitrary number of channels.
    void processPostFaderAndMix(
            GroupHandle inputHandle,
            GroupHandle outputHandle,
            CSAMPLE* pIn,
            CSAMPLE* pOut,
            const unsigned int numSamples,
            const unsigned int sampleRate,
            const GroupFeatureState& groupFeatures,
            const CSAMPLE_GAIN oldGain = CSAMPLE_GAIN_ONE,
            const CSAMPLE_GAIN newGain = CSAMPLE_GAIN_ONE);

    bool processEffectsRequest(
            EffectsRequest& message,
            EffectsResponsePipe* pResponsePipe) override;

  private:
    QString debugString() const {
        return QString("EngineEffectsManager");
    }

    bool addEffectChain(EngineEffectChain* pChain, SignalProcessingStage stage);
    bool removeEffectChain(EngineEffectChain* pChain, SignalProcessingStage stage);

    // Take a buffer of numSamples samples of audio from a channel, provided as
    // pInput, and apply each EngineEffectChain enabled for this channel to it,
    // putting the resulting output in pOutput. If pInput is equal to pOutput,
    // then the operation must occur in-place. Both pInput and pOutput are
    // represented as stereo interleaved samples. There are numSamples total
    // samples, so numSamples/2 left channel samples and numSamples/2 right
    // channel samples.
    void processInner(const SignalProcessingStage stage,
            GroupHandle inputHandle,
            GroupHandle outputHandle,
            CSAMPLE* pIn,
            CSAMPLE* pOut,
            const unsigned int numSamples,
            const unsigned int sampleRate,
            const GroupFeatureState& groupFeatures,
            const CSAMPLE_GAIN oldGain = CSAMPLE_GAIN_ONE,
            const CSAMPLE_GAIN newGain = CSAMPLE_GAIN_ONE);

    QScopedPointer<EffectsResponsePipe> m_pResponsePipe;
    QHash<SignalProcessingStage, QList<EngineEffectChain*>> m_chainsByStage;
    QList<EngineEffect*> m_effects;

    mixxx::SampleBuffer m_buffer1;
    mixxx::SampleBuffer m_buffer2;
};
