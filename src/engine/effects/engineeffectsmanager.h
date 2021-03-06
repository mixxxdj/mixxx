#pragma once

#include <QScopedPointer>

#include "engine/channelhandle.h"
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
class EngineEffectsManager final : public EffectsRequestHandler {
  public:
    EngineEffectsManager(EffectsResponsePipe* pResponsePipe);
    ~EngineEffectsManager();

    void onCallbackStart();

    // Take a buffer of numSamples samples of audio from a channel, provided as
    // pInput, and apply each EffectChain enabled for this channel to it,
    // putting the resulting output in pOutput. If pInput is equal to pOutput,
    // then the operation must occur in-place. Both pInput and pOutput are
    // represented as stereo interleaved samples. There are numSamples total
    // samples, so numSamples/2 left channel samples and numSamples/2 right
    // channel samples.
    void processPreFaderInPlace(
            const ChannelHandle& inputHandle,
            const ChannelHandle& outputHandle,
            CSAMPLE* pInOut,
            const unsigned int numSamples,
            const unsigned int sampleRate);

    void processPostFaderInPlace(
            const ChannelHandle& inputHandle,
            const ChannelHandle& outputHandle,
            CSAMPLE* pInOut,
            const unsigned int numSamples,
            const unsigned int sampleRate,
            const GroupFeatureState& groupFeatures,
            const CSAMPLE_GAIN oldGain = CSAMPLE_GAIN_ONE,
            const CSAMPLE_GAIN newGain = CSAMPLE_GAIN_ONE);

    void processPostFaderAndMix(
            const ChannelHandle& inputHandle,
            const ChannelHandle& outputHandle,
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

    void processInner(const SignalProcessingStage stage,
            const ChannelHandle& inputHandle,
            const ChannelHandle& outputHandle,
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
