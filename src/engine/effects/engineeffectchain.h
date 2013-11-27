#ifndef ENGINEEFFECTCHAIN_H
#define ENGINEEFFECTCHAIN_H

#include "defs.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/message.h"

class EngineEffectChain : public EffectsRequestHandler {
  public:
    EngineEffectChain(const QString& id);
    virtual ~EngineEffectChain();

    bool addEffect(EngineEffect* pEffect);
    bool removeEffect(EngineEffect* pEffect);

    bool processEffectsRequest(
        const EffectsRequest& message,
        const QSharedPointer<EffectsResponsePipe>& pResponsePipe);

    void process(const QString channelId,
                 const CSAMPLE* pInput, CSAMPLE* pOutput,
                 const unsigned int numSamples);

  private:
    QString debugString() const {
        return QString("EngineEffectChain(%1)").arg(m_id);
    }

    QString m_id;
    bool m_bEnabled;
    double m_dMix;
    double m_dParameter;
    QList<EngineEffect*> m_effects;
};

#endif /* ENGINEEFFECTCHAIN_H */
