#pragma once

#include <QList>
#include <QString>

#include "engine/channelhandle.h"
#include "engine/effects/groupfeaturestate.h"
#include "engine/effects/message.h"
#include "util/class.h"
#include "util/memory.h"
#include "util/samplebuffer.h"
#include "util/types.h"

class EngineEffect;

class EngineEffectChain : public EffectsRequestHandler {
  public:
    EngineEffectChain(const QString& group,
            const QSet<ChannelHandleAndGroup>& registeredInputChannels,
            const QSet<ChannelHandleAndGroup>& registeredOutputChannels);
    virtual ~EngineEffectChain();

    bool processEffectsRequest(
            EffectsRequest& message,
            EffectsResponsePipe* pResponsePipe);

    bool process(const ChannelHandle& inputHandle,
            const ChannelHandle& outputHandle,
            CSAMPLE* pIn,
            CSAMPLE* pOut,
            const unsigned int numSamples,
            const unsigned int sampleRate,
            const GroupFeatureState& groupFeatures);

    bool enabledForChannel(const ChannelHandle& handle) const;

    void deleteStatesForInputChannel(const ChannelHandle* channel);

  private:
    struct ChannelStatus {
        ChannelStatus()
                : oldMixKnob(0),
                  enableState(EffectEnableState::Disabled) {
        }
        CSAMPLE oldMixKnob;
        EffectEnableState enableState;
    };

    QString debugString() const {
        return QString("EngineEffectChain(%1)").arg(m_group);
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

    QString m_group;
    EffectEnableState m_enableState;
    EffectChainMixMode m_mixMode;
    CSAMPLE m_dMix;
    QList<EngineEffect*> m_effects;
    mixxx::SampleBuffer m_buffer1;
    mixxx::SampleBuffer m_buffer2;
    ChannelHandleMap<ChannelHandleMap<ChannelStatus>> m_chainStatusForChannelMatrix;

    DISALLOW_COPY_AND_ASSIGN(EngineEffectChain);
};
