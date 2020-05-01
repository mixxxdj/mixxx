#pragma once

#include "engine/effects/message.h"

/// EffectsMessenger sends EffectsRequest and receives EffectsResponses
class EffectsMessenger {
  public:
    EffectsMessenger(EffectsRequestPipe* pRequestPipe, EffectsResponsePipe* m_pResponsePipe);
    ~EffectsMessenger();
    /// Write an EffectsRequest to the EngineEffectsManager. EffectsMessenger takes
    /// ownership of request and deletes it once a response is received.
    bool writeRequest(EffectsRequest* request);

    void startShutdownProcess();
    void processEffectsResponses();

  private:
    void collectGarbage(const EffectsRequest* pRequest);

    QString debugString() const {
        return "EffectsMessenger";
    }

    bool m_bShuttingDown;

    QScopedPointer<EffectsRequestPipe> m_pRequestPipe;
    QScopedPointer<EffectsResponsePipe> m_pResponsePipe;
    qint64 m_nextRequestId;
    QHash<qint64, EffectsRequest*> m_activeRequests;
};
