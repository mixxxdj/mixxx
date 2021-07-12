#pragma once

#include <QList>

#include "engine/channelhandle.h"
#include "engine/effects/message.h"
#include "engine/effects/groupfeaturestate.h"
#include "util/samplebuffer.h"

class EngineEffectChain;

//TODO(Be): Remove this superfluous class.
class EngineEffectRack : public EffectsRequestHandler {
  public:
    EngineEffectRack(int iRackNumber);
    virtual ~EngineEffectRack();

    bool processEffectsRequest(
        EffectsRequest& message,
        EffectsResponsePipe* pResponsePipe);

    bool process(const ChannelHandle& inputHandle,
                 const ChannelHandle& outputHandle,
                 CSAMPLE* pIn, CSAMPLE* pOut,
                 const unsigned int numSamples,
                 const unsigned int sampleRate,
                 const GroupFeatureState& groupFeatures);

    int number() const {
        return m_iRackNumber;
    }

  private:
    bool addEffectChain(EngineEffectChain* pChain, int iIndex);
    bool removeEffectChain(EngineEffectChain* pChain, int iIndex);

    QString debugString() const {
        return QString("EngineEffectRack%1").arg(m_iRackNumber);
    }

    int m_iRackNumber;
    QList<EngineEffectChain*> m_chains;

    mixxx::SampleBuffer m_buffer1;
    mixxx::SampleBuffer m_buffer2;

    DISALLOW_COPY_AND_ASSIGN(EngineEffectRack);
};
