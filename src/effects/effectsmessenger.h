#pragma once

#include "engine/effects/message.h"

/// EffectsMessenger sends EffectsRequests from the main thread and receives
/// EffectsResponses from the audio thread. This allows memory allocation and
/// deallocation on the heap, which is slow, to be done in the main thread to
/// avoid blocking the audio thread and causing audible glitches. All of
/// EffectsMessenger's methods are called on the main thread.
/// Refer to
/// http://www.rossbencina.com/code/real-time-audio-programming-101-time-waits-for-nothing
/// for background information and
/// https://github.com/mixxxdj/mixxx/pull/180#issuecomment-37435684
/// for why this design is used for effects rather than alternatives.
class EffectsMessenger {
  public:
    // passing by rvalue-ref because we want to ensure we're the only on with access to that pipe
    EffectsMessenger(EffectsRequestPipe&& requestPipe);
    ~EffectsMessenger();
    /// Write an EffectsRequest to the EngineEffectsManager. EffectsMessenger takes
    /// ownership of request and deletes it once a response is received.
    bool writeRequest(EffectsRequest* request);

    void initiateShutdown();
    void processEffectsResponses();

  private:
    void collectGarbage(const EffectsRequest* pRequest);

    QString debugString() const {
        return "EffectsMessenger";
    }

    QHash<qint64, EffectsRequest*> m_activeRequests;
    EffectsRequestPipe m_requestPipe;
    qint64 m_nextRequestId;
    bool m_bShuttingDown;
};
