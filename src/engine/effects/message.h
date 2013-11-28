#ifndef MESSAGE_H
#define MESSAGE_H

#include "util/fifo.h"

class EngineEffectChain;
class EngineEffect;

struct EffectsRequest {
    enum MessageType {
        // Messages for EngineEffectsManager
        ADD_EFFECT_CHAIN_SLOT,
        REMOVE_EFFECT_CHAIN_SLOT,
        ADD_EFFECT_CHAIN,
        REMOVE_EFFECT_CHAIN,

        // Messages for EngineEffectChain
        SET_EFFECT_CHAIN_PARAMETERS,
        ADD_EFFECT_TO_CHAIN,
        REMOVE_EFFECT_FROM_CHAIN,

        // Must come last.
        NUM_REQUEST_TYPES
    };

    EffectsRequest()
            : type(NUM_REQUEST_TYPES),
              request_id(-1) {
        memset(&AddEffectChain, 0, sizeof(AddEffectChain));
        memset(&RemoveEffectChain, 0, sizeof(RemoveEffectChain));
        memset(&AddEffectToChain, 0, sizeof(AddEffectToChain));
        memset(&RemoveEffectFromChain, 0, sizeof(RemoveEffectFromChain));
        memset(&SetEffectChainParameters, 0, sizeof(SetEffectChainParameters));
    }

    MessageType type;
    qint64 request_id;

    // The chain referred to by this request. Not POD so can't be part of the
    // union below.
    QString chainId;

    union {
        struct {
            EngineEffectChain* pChain;
        } AddEffectChain;
        struct {
            EngineEffectChain* pChain;
        } RemoveEffectChain;

        struct {
            // The effect referred to by this request. NULL if none.
            EngineEffect* pEffect;
        } AddEffectToChain;
        struct {
            // The effect referred to by this request. NULL if none.
            EngineEffect* pEffect;
        } RemoveEffectFromChain;
        struct {
            bool enabled;
            double mix;
            double parameter;
        } SetEffectChainParameters;
    };
};

struct EffectsResponse {
    enum StatusCode {
        OK,
        UNHANDLED_MESSAGE_TYPE,
        NO_SUCH_CHAIN,

        // Must come last.
        NUM_STATUS_CODES
    };
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
              success(false),
              status(NUM_STATUS_CODES) {
    }

    EffectsResponse(const EffectsRequest& request, bool succeeded=false)
            : type(NUM_RESPONSE_TYPES),
              request_id(request.request_id),
              success(succeeded),
              status(NUM_STATUS_CODES) {
    }

    MessageType type;
    qint64 request_id;
    bool success;
    StatusCode status;
};

// For communicating from the main thread to the EngineEffectsManager.
typedef MessagePipe<EffectsRequest, EffectsResponse> EffectsRequestPipe;

// For communicating from the EngineEffectsManager to the main thread.
typedef MessagePipe<EffectsResponse, EffectsRequest> EffectsResponsePipe;

class EffectsRequestHandler {
  public:
    virtual bool processEffectsRequest(
        const EffectsRequest& message,
        EffectsResponsePipe* pResponsePipe) = 0;
};

#endif /* MESSAGE_H */
