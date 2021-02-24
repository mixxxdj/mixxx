#pragma once

#include <QScopedPointer>

#include "util/samplebuffer.h"
#include "util/types.h"
#include "util/fifo.h"
#include "engine/effects/message.h"
#include "engine/effects/groupfeaturestate.h"
#include "engine/channelhandle.h"

class EngineEffectRack;
class EngineEffectChain;
class EngineEffect;

class EngineEffectsManager : public EffectsRequestHandler {
  public:
    EngineEffectsManager(EffectsResponsePipe* pResponsePipe);
    virtual ~EngineEffectsManager();

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
        CSAMPLE* pIn, CSAMPLE* pOut,
        const unsigned int numSamples,
        const unsigned int sampleRate,
        const GroupFeatureState& groupFeatures,
        const CSAMPLE_GAIN oldGain = CSAMPLE_GAIN_ONE,
        const CSAMPLE_GAIN newGain = CSAMPLE_GAIN_ONE);

    bool processEffectsRequest(
        EffectsRequest& message,
        EffectsResponsePipe* pResponsePipe);

  private:
    QString debugString() const {
        return QString("EngineEffectsManager");
    }

    bool addEffectRack(EngineEffectRack* pRack, SignalProcessingStage stage);
    bool removeEffectRack(EngineEffectRack* pRack, SignalProcessingStage stage);

    bool addPreFaderEffectRack(EngineEffectRack* pRack);
    bool removePreFaderEffectRack(EngineEffectRack* pRack);

    bool addPostFaderEffectRack(EngineEffectRack* pRack);
    bool removePostFaderEffectRack(EngineEffectRack* pRack);

    void processInner(const SignalProcessingStage stage,
                      const ChannelHandle& inputHandle,
                      const ChannelHandle& outputHandle,
                      CSAMPLE* pIn, CSAMPLE* pOut,
                      const unsigned int numSamples,
                      const unsigned int sampleRate,
                      const GroupFeatureState& groupFeatures,
                      const CSAMPLE_GAIN oldGain = CSAMPLE_GAIN_ONE,
                      const CSAMPLE_GAIN newGain = CSAMPLE_GAIN_ONE);

    QScopedPointer<EffectsResponsePipe> m_pResponsePipe;
    QHash<SignalProcessingStage, QList<EngineEffectRack*>> m_racksByStage;
    QList<EngineEffectChain*> m_chains;
    QList<EngineEffect*> m_effects;

    mixxx::SampleBuffer m_buffer1;
    mixxx::SampleBuffer m_buffer2;
};
