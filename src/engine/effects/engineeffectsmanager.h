#ifndef ENGINEEFFECTSMANAGER_H
#define ENGINEEFFECTSMANAGER_H

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
        const GroupFeatureState& groupFeatures);

    void processPostFaderAndMix(
        const ChannelHandle& inputHandle,
        const ChannelHandle& outputHandle,
        CSAMPLE* pIn, CSAMPLE* pOut,
        const unsigned int numSamples,
        const unsigned int sampleRate,
        const GroupFeatureState& groupFeatures,
        const CSAMPLE_GAIN oldGain, const CSAMPLE_GAIN newGain);

    bool processEffectsRequest(
        const EffectsRequest& message,
        EffectsResponsePipe* pResponsePipe);

  private:
    QString debugString() const {
        return QString("EngineEffectsManager");
    }

    bool addPreFaderEffectRack(EngineEffectRack* pRack);
    bool removePreFaderEffectRack(EngineEffectRack* pRack);

    bool addPostFaderEffectRack(EngineEffectRack* pRack);
    bool removePostFaderEffectRack(EngineEffectRack* pRack);

    void processInner(const QList<EngineEffectRack*>& racks,
                      const ChannelHandle& inputHandle,
                      const ChannelHandle& outputHandle,
                      CSAMPLE* pIn, CSAMPLE* pOut,
                      const unsigned int numSamples,
                      const unsigned int sampleRate,
                      const GroupFeatureState& groupFeatures,
                      const CSAMPLE_GAIN oldGain = 0, const CSAMPLE_GAIN newGain = 0);

    QScopedPointer<EffectsResponsePipe> m_pResponsePipe;
    QList<EngineEffectRack*> m_preFaderRacks;
    QList<EngineEffectRack*> m_postFaderRacks;
    QList<EngineEffectChain*> m_chains;
    QList<EngineEffect*> m_effects;

    SampleBuffer m_buffer1;
    SampleBuffer m_buffer2;
};


#endif /* ENGINEEFFECTSMANAGER_H */
