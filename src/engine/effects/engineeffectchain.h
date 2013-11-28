#ifndef ENGINEEFFECTCHAIN_H
#define ENGINEEFFECTCHAIN_H

#include "defs.h"
#include "engine/effects/message.h"

class EngineEffect;

class EngineEffectChain : public EffectsRequestHandler {
  public:
    EngineEffectChain(const QString& id);
    virtual ~EngineEffectChain();

    bool processEffectsRequest(
        const EffectsRequest& message,
        const QSharedPointer<EffectsResponsePipe>& pResponsePipe);

    void process(const QString channelId,
                 const CSAMPLE* pInput, CSAMPLE* pOutput,
                 const unsigned int numSamples);

    const QString& id() const {
        return m_id;
    }

  private:
    QString debugString() const {
        return QString("EngineEffectChain(%1)").arg(m_id);
    }

    bool updateParameters(const EffectsRequest& message);
    bool addEffect(EngineEffect* pEffect);
    bool removeEffect(EngineEffect* pEffect);

    QString m_id;
    bool m_bEnabled;
    double m_dMix;
    double m_dParameter;
    QList<EngineEffect*> m_effects;
};

#endif /* ENGINEEFFECTCHAIN_H */
