#ifndef ENGINEEFFECTCHAIN_H
#define ENGINEEFFECTCHAIN_H

#include <QString>
#include <QList>
#include <QLinkedList>

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
    struct GroupStatus {
        GroupStatus(const QString& group)
                : group(group),
                  old_gain(0),
                  enable_state(EffectProcessor::DISABLED) {
        }
        QString group;
        CSAMPLE old_gain;
        EffectProcessor::EnableState enable_state;
    };

    QString debugString() const {
        return QString("EngineEffectChain(%1)").arg(m_id);
    }

    bool updateParameters(const EffectsRequest& message);
    bool addEffect(EngineEffect* pEffect, int iIndex);
    bool removeEffect(EngineEffect* pEffect, int iIndex);
    bool enableForGroup(const QString& group);
    bool disableForGroup(const QString& group);

    // Gets or creates a GroupStatus entry in m_groupStatus for the provided
    // group.
    GroupStatus& getGroupStatus(const QString& group);

    QString m_id;
    EffectProcessor::EnableState m_enableState;
    EffectChain::InsertionType m_insertionType;
    CSAMPLE m_dMix;
    QList<EngineEffect*> m_effects;
    CSAMPLE* m_pBuffer;
    QLinkedList<GroupStatus> m_groupStatus;

    DISALLOW_COPY_AND_ASSIGN(EngineEffectChain);
};

#endif /* ENGINEEFFECTCHAIN_H */
