#include "effects/effectsmessenger.h"

#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectchain.h"
#include "util/make_const_iterator.h"

EffectsMessenger::EffectsMessenger(
        EffectsRequestPipe&& requestPipe)
        : m_requestPipe(std::move(requestPipe)),
          m_nextRequestId(0),
          m_bShuttingDown(false) {
}

EffectsMessenger::~EffectsMessenger() {
    for (auto it = m_activeRequests.begin(); it != m_activeRequests.end(); it++) {
        delete it.value();
    }
}

void EffectsMessenger::initiateShutdown() {
    m_bShuttingDown = true;
}

bool EffectsMessenger::writeRequest(EffectsRequest* request) {
    if (m_bShuttingDown) {
        // Catch all delete Messages since the engine is already down
        // and we cannot wait for a communication cycle
        collectGarbage(request);
    }

    // This is effectively only garbage collection at this point so only deal
    // with responses when writing new requests.
    processEffectsResponses();

    request->request_id = m_nextRequestId++;
    // TODO(XXX) use preallocated requests to avoid delete calls from engine
    if (m_requestPipe.writeMessage(request)) {
        m_activeRequests[request->request_id] = request;
        return true;
    }
    delete request;
    return false;
}

void EffectsMessenger::processEffectsResponses() {
    EffectsResponse response;
    while (m_requestPipe.readMessage(&response)) {
        auto it = m_activeRequests.constFind(response.request_id);

        VERIFY_OR_DEBUG_ASSERT(it != m_activeRequests.constEnd()) {
            qWarning() << debugString()
                       << "WARNING: EffectsResponse with an inactive request_id:"
                       << response.request_id;
        }

        while (it != m_activeRequests.constEnd() &&
                it.key() == response.request_id) {
            EffectsRequest* pRequest = it.value();

            // Don't check whether the response was successful here because
            // specific errors should be caught with DEBUG_ASSERTs in
            // EngineEffectsMessenger and functions it calls to handle requests.

            collectGarbage(pRequest);

            delete pRequest;
            it = constErase(&m_activeRequests, it);
        }
    }
}

void EffectsMessenger::collectGarbage(const EffectsRequest* pRequest) {
    if (pRequest->type == EffectsRequest::REMOVE_EFFECT_FROM_CHAIN) {
        if (kEffectDebugOutput) {
            qDebug() << debugString() << "delete" << pRequest->RemoveEffectFromChain.pEffect;
        }
        delete pRequest->RemoveEffectFromChain.pEffect;
    } else if (pRequest->type == EffectsRequest::REMOVE_EFFECT_CHAIN) {
        if (kEffectDebugOutput) {
            qDebug() << debugString() << "delete" << pRequest->RemoveEffectChain.pChain;
        }
        delete pRequest->RemoveEffectChain.pChain;
    }
}
