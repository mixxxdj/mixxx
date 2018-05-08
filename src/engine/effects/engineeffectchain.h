#ifndef ENGINEEFFECTCHAIN_H
#define ENGINEEFFECTCHAIN_H

#include <QString>
#include <QList>
#include <QLinkedList>

#include "util/class.h"
#include "util/types.h"
#include "util/samplebuffer.h"
#include "util/memory.h"
#include "engine/channelhandle.h"
#include "engine/effects/message.h"
#include "engine/effects/groupfeaturestate.h"
#include "effects/effectchain.h"

class EngineEffect;

class EngineEffectChain : public EffectsRequestHandler {
  public:
    EngineEffectChain(const QString& id,
                      const QSet<ChannelHandleAndGroup>& registeredInputChannels,
                      const QSet<ChannelHandleAndGroup>& registeredOutputChannels);
    virtual ~EngineEffectChain();

    bool processEffectsRequest(
        EffectsRequest& message,
        EffectsResponsePipe* pResponsePipe);

    bool process(const ChannelHandle& inputHandle,
                 const ChannelHandle& outputHandle,
                 CSAMPLE* pIn, CSAMPLE* pOut,
                 const unsigned int numSamples,
                 const unsigned int sampleRate,
                 const GroupFeatureState& groupFeatures);

    const QString& id() const {
        return m_id;
    }

    bool enabledForChannel(const ChannelHandle& handle) const;

    void deleteStatesForInputChannel(const ChannelHandle* channel);

  private:
    struct ChannelStatus {
        ChannelStatus()
                : old_gain(0),
                  enable_state(EffectEnableState::Disabled) {
        }
        CSAMPLE old_gain;
        EffectEnableState enable_state;
    };

    QString debugString() const {
        return QString("EngineEffectChain(%1)").arg(m_id);
    }

    bool updateParameters(const EffectsRequest& message);
    bool addEffect(EngineEffect* pEffect, int iIndex);
    bool removeEffect(EngineEffect* pEffect, int iIndex);
    bool enableForInputChannel(const ChannelHandle* inputHandle,
            EffectStatesMapArray* statesForEffectsInChain);
    bool disableForInputChannel(const ChannelHandle* inputHandle);

    // Gets or creates a ChannelStatus entry in m_channelStatus for the provided
    // handle.
    ChannelStatus& getChannelStatus(const ChannelHandle& inputHandle,
                                    const ChannelHandle& outputHandle);

    QString m_id;
    EffectEnableState m_enableState;
    EffectChainInsertionType m_insertionType;
    CSAMPLE m_dMix;
    QList<EngineEffect*> m_effects;
    mixxx::SampleBuffer m_buffer1;
    mixxx::SampleBuffer m_buffer2;
    ChannelHandleMap<ChannelHandleMap<ChannelStatus>> m_chainStatusForChannelMatrix;

    DISALLOW_COPY_AND_ASSIGN(EngineEffectChain);
};

#endif /* ENGINEEFFECTCHAIN_H */
