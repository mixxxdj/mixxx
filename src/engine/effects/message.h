#ifndef MESSAGE_H
#define MESSAGE_H

#include "util/fifo.h"

class EngineEffect;

struct EffectsRequest {
    enum MessageType {
        ADD_EFFECT_CHAIN_SLOT,
        REMOVE_EFFECT_CHAIN_SLOT,
        ADD_EFFECT_CHAIN,
        REMOVE_EFFECT_CHAIN,
        ADD_EFFECT_TO_CHAIN,
        REMOVE_EFFECT_FROM_CHAIN,

        // Must come last.
        NUM_REQUEST_TYPES
    };

    EffectsRequest()
            : type(NUM_REQUEST_TYPES),
              request_id(-1),
              pEffect(NULL) {
    }

    MessageType type;
    qint64 request_id;

    // The chain referred to by this request.
    QString chainId;

    // The effect referred to by this request. NULL if none.
    EngineEffect* pEffect;
};

struct EffectsResponse {
    enum MessageType {
        ADD_EFFECT_CHAIN_SLOT,
        REMOVE_EFFECT_CHAIN_SLOT,
        ADD_EFFECT_CHAIN,
        REMOVE_EFFECT_CHAIN,
        ADD_EFFECT_TO_CHAIN,
        REMOVE_EFFECT_FROM_CHAIN,

        // Must come last.
        NUM_RESPONSE_TYPES
    };

    EffectsResponse()
            : type(NUM_RESPONSE_TYPES),
              request_id(-1),
              success(false) {
    }

    EffectsResponse(const EffectsRequest& request, bool succeeded=false)
            : type(NUM_RESPONSE_TYPES),
              request_id(request.request_id),
              success(succeeded) {
    }

    MessageType type;
    qint64 request_id;
    bool success;
};

// For communicating from the main thread to the EngineEffectsManager.
typedef MessagePipe<EffectsRequest, EffectsResponse> EffectsRequestPipe;

// For communicating from the EngineEffectsManager to the main thread.
typedef MessagePipe<EffectsResponse, EffectsRequest> EffectsResponsePipe;

class EffectsRequestHandler {
  public:
    virtual bool processEffectsRequest(
        const EffectsRequest& message,
        const QSharedPointer<EffectsResponsePipe>& pResponsePipe) = 0;
};

#endif /* MESSAGE_H */
