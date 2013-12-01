#ifndef MESSAGE_H
#define MESSAGE_H

#include <QVariant>
#include <QString>
#include <QtGlobal>

#include "util/fifo.h"

class EngineEffectChain;
class EngineEffect;

struct EffectsRequest {
    enum MessageType {
        // Messages for EngineEffectsManager
        ADD_EFFECT_CHAIN,
        REMOVE_EFFECT_CHAIN,

        // Messages for EngineEffectChain
        SET_EFFECT_CHAIN_PARAMETERS,
        ADD_EFFECT_TO_CHAIN,
        REMOVE_EFFECT_FROM_CHAIN,
        ENABLE_EFFECT_CHAIN_FOR_GROUP,
        DISABLE_EFFECT_CHAIN_FOR_GROUP,

        // Messages for EngineEffect
        SET_EFFECT_PARAMETER,

        // Must come last.
        NUM_REQUEST_TYPES
    };

    EffectsRequest()
            : type(NUM_REQUEST_TYPES),
              request_id(-1) {
        pTargetChain = NULL;
        pTargetEffect = NULL;
#define CLEAR_STRUCT(x) memset(&x, 0, sizeof(x));
        CLEAR_STRUCT(AddEffectChain);
        CLEAR_STRUCT(RemoveEffectChain);
        CLEAR_STRUCT(AddEffectToChain);
        CLEAR_STRUCT(RemoveEffectFromChain);
        CLEAR_STRUCT(SetEffectChainParameters);
        CLEAR_STRUCT(SetEffectParameter);
#undef CLEAR_STRUCT
    }

    MessageType type;
    qint64 request_id;

    // Target of the message.
    union {
        // Used by:
        // - ADD_EFFECT_TO_CHAIN
        // - REMOVE_EFFECT_FROM_CHAIN
        // - SET_EFFECT_CHAIN_PARAMETERS
        // - ENABLE_EFFECT_CHAIN_FOR_GROUP
        // - DISABLE_EFFECT_CHAIN_FOR_GROUP
        EngineEffectChain* pTargetChain;
        // Used by:
        // - SET_EFFECT_PARAMETER
        EngineEffect* pTargetEffect;
    };

    // Message-specific data.
    union {
        struct {
            EngineEffectChain* pChain;
        } AddEffectChain;
        struct {
            EngineEffectChain* pChain;
        } RemoveEffectChain;
        struct {
            EngineEffect* pEffect;
            int iIndex;
        } AddEffectToChain;
        struct {
            EngineEffect* pEffect;
        } RemoveEffectFromChain;
        struct {
            bool enabled;
            double mix;
            double parameter;
        } SetEffectChainParameters;
        struct {
            int iParameter;
        } SetEffectParameter;
    };

    ////////////////////////////////////////////////////////////////////////////
    // Message-specific, non-POD values that can't be part of the above union.
    ////////////////////////////////////////////////////////////////////////////

    // Used by ENABLE_EFFECT_CHAIN_FOR_GROUP and DISABLE_EFFECT_CHAIN_FOR_GROUP.
    QString group;

    // Used by SET_EFFECT_PARAMETER.
    QVariant minimum;
    QVariant maximum;
    QVariant default_value;
    QVariant value;
};

struct EffectsResponse {
    enum StatusCode {
        OK,
        UNHANDLED_MESSAGE_TYPE,
        NO_SUCH_CHAIN,
        NO_SUCH_EFFECT,
        NO_SUCH_PARAMETER,
        INVALID_REQUEST,

        // Must come last.
        NUM_STATUS_CODES
    };

    EffectsResponse()
            : request_id(-1),
              success(false),
              status(NUM_STATUS_CODES) {
    }

    EffectsResponse(const EffectsRequest& request, bool succeeded=false)
            : request_id(request.request_id),
              success(succeeded),
              status(NUM_STATUS_CODES) {
    }

    qint64 request_id;
    bool success;
    StatusCode status;
};

// For communicating from the main thread to the EngineEffectsManager.
typedef MessagePipe<EffectsRequest*, EffectsResponse> EffectsRequestPipe;

// For communicating from the EngineEffectsManager to the main thread.
typedef MessagePipe<EffectsResponse, EffectsRequest*> EffectsResponsePipe;

class EffectsRequestHandler {
  public:
    virtual bool processEffectsRequest(
        const EffectsRequest& message,
        EffectsResponsePipe* pResponsePipe) = 0;
};

#endif /* MESSAGE_H */
