#ifndef MESSAGE_H
#define MESSAGE_H

#include <QVariant>
#include <QString>
#include <QtGlobal>

#include "util/fifo.h"
#include "effects/effectchain.h"

const bool kEffectDebugOutput = false;

class EngineEffectRack;
class EngineEffectChain;
class EngineEffect;

struct EffectsRequest {
    enum MessageType {
        // Messages for EngineEffectsManager
        ADD_EFFECT_RACK = 0,
        REMOVE_EFFECT_RACK,

        // Messages for EngineEffectRack
        ADD_CHAIN_TO_RACK,
        REMOVE_CHAIN_FROM_RACK,

        // Messages for EngineEffectChain
        SET_EFFECT_CHAIN_PARAMETERS,
        ADD_EFFECT_TO_CHAIN,
        REMOVE_EFFECT_FROM_CHAIN,
        ENABLE_EFFECT_CHAIN_FOR_CHANNEL,
        DISABLE_EFFECT_CHAIN_FOR_CHANNEL,

        // Messages for EngineEffect
        SET_EFFECT_PARAMETERS,
        SET_PARAMETER_PARAMETERS,

        // Must come last.
        NUM_REQUEST_TYPES
    };

    EffectsRequest()
            : type(NUM_REQUEST_TYPES),
              request_id(-1),
              minimum(0.0),
              maximum(0.0),
              default_value(0.0),
              value(0.0) {
        pTargetRack = NULL;
        pTargetChain = NULL;
        pTargetEffect = NULL;
#define CLEAR_STRUCT(x) memset(&x, 0, sizeof(x));
        CLEAR_STRUCT(AddEffectRack);
        CLEAR_STRUCT(RemoveEffectRack);
        CLEAR_STRUCT(AddChainToRack);
        CLEAR_STRUCT(RemoveChainFromRack);
        CLEAR_STRUCT(AddEffectToChain);
        CLEAR_STRUCT(RemoveEffectFromChain);
        CLEAR_STRUCT(SetEffectChainParameters);
        CLEAR_STRUCT(SetEffectParameters);
        CLEAR_STRUCT(SetParameterParameters);
#undef CLEAR_STRUCT
    }

    MessageType type;
    qint64 request_id;

    // Target of the message.
    union {
        // Used by:
        // - ADD_CHAIN_TO_RACK
        // - REMOVE_CHAIN_FROM_RACK
        EngineEffectRack* pTargetRack;
        // Used by:
        // - ADD_EFFECT_TO_CHAIN
        // - REMOVE_EFFECT_FROM_CHAIN
        // - SET_EFFECT_CHAIN_PARAMETERS
        // - ENABLE_EFFECT_CHAIN_FOR_CHANNEL
        // - DISABLE_EFFECT_CHAIN_FOR_CHANNEL
        EngineEffectChain* pTargetChain;
        // Used by:
        // - SET_EFFECT_PARAMETER
        EngineEffect* pTargetEffect;
    };

    // Message-specific data.
    union {
        struct {
            EngineEffectRack* pRack;
        } AddEffectRack;
        struct {
            EngineEffectRack* pRack;
        } RemoveEffectRack;
        struct {
            EngineEffectChain* pChain;
            int iIndex;
        } AddChainToRack;
        struct {
            EngineEffectChain* pChain;
            int iIndex;
        } RemoveChainFromRack;
        struct {
            EngineEffect* pEffect;
            int iIndex;
        } AddEffectToChain;
        struct {
            EngineEffect* pEffect;
            int iIndex;
        } RemoveEffectFromChain;
        struct {
            bool enabled;
            EffectChain::InsertionType insertion_type;
            double mix;
        } SetEffectChainParameters;
        struct {
            bool enabled;
        } SetEffectParameters;
        struct {
            int iParameter;
        } SetParameterParameters;
    };

    ////////////////////////////////////////////////////////////////////////////
    // Message-specific, non-POD values that can't be part of the above union.
    ////////////////////////////////////////////////////////////////////////////

    // Used by ENABLE_EFFECT_CHAIN_FOR_CHANNEL and DISABLE_EFFECT_CHAIN_FOR_CHANNEL.
    ChannelHandle channel;

    // Used by SET_EFFECT_PARAMETER.
    double minimum;
    double maximum;
    double default_value;
    double value;
};

struct EffectsResponse {
    enum StatusCode {
        OK,
        UNHANDLED_MESSAGE_TYPE,
        NO_SUCH_RACK,
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
