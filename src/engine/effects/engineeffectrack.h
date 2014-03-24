#ifndef ENGINEEFFECTRACK_H
#define ENGINEEFFECTRACK_H

#include <QList>

#include "engine/effects/message.h"

class EngineEffectChain;

class EngineEffectRack : public EffectsRequestHandler {
  public:
    EngineEffectRack(int iRackNumber);
    virtual ~EngineEffectRack();

    bool processEffectsRequest(
        const EffectsRequest& message,
        EffectsResponsePipe* pResponsePipe);

    void process(const QString& group,
                 const CSAMPLE* pInput, CSAMPLE* pOutput,
                 const unsigned int numSamples);

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

    DISALLOW_COPY_AND_ASSIGN(EngineEffectRack);
};

#endif /* ENGINEEFFECTRACK_H */
