#ifndef ENGINEEFFECTSMANAGER_H
#define ENGINEEFFECTSMANAGER_H

#include <QScopedPointer>

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
    virtual void process(const ChannelHandle& handle,
                         CSAMPLE* pInOut,
                         const unsigned int numSamples,
                         const unsigned int sampleRate,
                         const GroupFeatureState& groupFeatures);

    bool processEffectsRequest(
        const EffectsRequest& message,
        EffectsResponsePipe* pResponsePipe);

  private:
    QString debugString() const {
        return QString("EngineEffectsManager");
    }

    bool addEffectRack(EngineEffectRack* pRack);
    bool removeEffectRack(EngineEffectRack* pRack);

    QScopedPointer<EffectsResponsePipe> m_pResponsePipe;
    QList<EngineEffectRack*> m_racks;
    QList<EngineEffectChain*> m_chains;
    QList<EngineEffect*> m_effects;
};


#endif /* ENGINEEFFECTSMANAGER_H */
