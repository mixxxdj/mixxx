#ifndef ENGINEEFFECTCHAIN_H
#define ENGINEEFFECTCHAIN_H

#include <QString>
#include <QList>
#include <QSet>

#include "defs.h"
#include "engine/effects/message.h"

class EngineEffect;

class EngineEffectChain : public EffectsRequestHandler {
  public:
    EngineEffectChain(const QString& id);
    virtual ~EngineEffectChain();

    bool processEffectsRequest(
        const EffectsRequest& message,
        EffectsResponsePipe* pResponsePipe);

    void process(const QString& group,
                 const CSAMPLE* pInput, CSAMPLE* pOutput,
                 const unsigned int numSamples);

    const QString& id() const {
        return m_id;
    }

    bool enabledForGroup(const QString& group) const;

  private:
    QString debugString() const {
        return QString("EngineEffectChain(%1)").arg(m_id);
    }

    bool updateParameters(const EffectsRequest& message);
    bool addEffect(EngineEffect* pEffect, int iIndex);
    bool removeEffect(EngineEffect* pEffect);
    bool enableForGroup(const QString& group);
    bool disableForGroup(const QString& group);

    QString m_id;
    bool m_bEnabled;
    double m_dMix;
    double m_dParameter;
    QList<EngineEffect*> m_effects;
    QSet<QString> m_enabledGroups;
};

#endif /* ENGINEEFFECTCHAIN_H */
