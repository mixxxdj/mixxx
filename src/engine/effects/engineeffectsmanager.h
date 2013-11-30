#ifndef ENGINEEFFECTSMANAGER_H
#define ENGINEEFFECTSMANAGER_H

#include <QScopedPointer>

#include "defs.h"
#include "util/fifo.h"
#include "engine/effects/message.h"

class EngineEffectChain;

class EngineEffectsManager : public EffectsRequestHandler {
  public:
    EngineEffectsManager(EffectsResponsePipe* pResponsePipe);
    virtual ~EngineEffectsManager();

    void onCallbackStart();

    // Take a buffer of numSamples samples of audio from channel channelId,
    // provided as pInput, and apply each EffectChain enabled for this channel
    // to it, putting the resulting output in pOutput. If pInput is equal to
    // pOutput, then the operation must occur in-place. Both pInput and pOutput
    // are represented as stereo interleaved samples. There are numSamples total
    // samples, so numSamples/2 left channel samples and numSamples/2 right
    // channel samples.
    virtual void process(const QString channelId,
                         const CSAMPLE* pInput, CSAMPLE* pOutput,
                         const unsigned int numSamples);

    bool processEffectsRequest(
        const EffectsRequest& message,
        EffectsResponsePipe* pResponsePipe);

  private:
    QString debugString() const {
        return QString("EngineEffectsManager");
    }

    bool addEffectChain(EngineEffectChain* pChain);
    bool removeEffectChain(EngineEffectChain* pChain);

    QScopedPointer<EffectsResponsePipe> m_pResponsePipe;
    QList<EngineEffectChain*> m_chains;
    QList<EngineEffect*> m_effects;
};


#endif /* ENGINEEFFECTSMANAGER_H */
