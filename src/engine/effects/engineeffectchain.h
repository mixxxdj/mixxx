#ifndef ENGINEEFFECTCHAIN_H
#define ENGINEEFFECTCHAIN_H

#include <QString>
#include <QList>
#include <QSet>

#include "util.h"
#include "util/types.h"
#include "engine/effects/message.h"
#include "engine/effects/groupfeaturestate.h"
#include "effects/effectchain.h"

class EngineEffect;

class EngineEffectChain : public EffectsRequestHandler {
  public:
    EngineEffectChain(const QString& id);
    virtual ~EngineEffectChain();

    bool processEffectsRequest(
        const EffectsRequest& message,
        EffectsResponsePipe* pResponsePipe);

    void process(const QString& group,
                 CSAMPLE* pInOut,
                 const unsigned int numSamples,
                 const unsigned int sampleRate,
                 const GroupFeatureState& groupFeatures);

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
    bool removeEffect(EngineEffect* pEffect, int iIndex);
    bool enableForGroup(const QString& group);
    bool disableForGroup(const QString& group);

    QString m_id;
    EffectProcessor::EnableState m_enableState;
    EffectChain::InsertionType m_insertionType;
    CSAMPLE m_dMix;
    QList<EngineEffect*> m_effects;
    CSAMPLE* m_pBuffer;
    struct GroupStatus {
        GroupStatus()
                : old_gain(0),
                  enable_state(EffectProcessor::DISABLED) {
        }
        CSAMPLE old_gain;
        EffectProcessor::EnableState enable_state;
    };
    QMap<QString, GroupStatus> m_groupStatus;

    DISALLOW_COPY_AND_ASSIGN(EngineEffectChain);
};

#endif /* ENGINEEFFECTCHAIN_H */
