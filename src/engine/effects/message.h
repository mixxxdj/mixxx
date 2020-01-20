#pragma once

#include <QVariant>
#include <QString>
#include <QtGlobal>

#include "util/memory.h"
#include "util/messagepipe.h"
#include "effects/defs.h"
#include "engine/channelhandle.h"

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
        // Effects cannot currently be toggled for output channels;
        // the outputs that effects are applied to are hardwired in EngineMaster
        ENABLE_EFFECT_CHAIN_FOR_INPUT_CHANNEL,
        DISABLE_EFFECT_CHAIN_FOR_INPUT_CHANNEL,

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
        pTargetRack = nullptr;
        pTargetChain = nullptr;
        pTargetEffect = nullptr;
#define CLEAR_STRUCT(x) memset(&x, 0, sizeof(x));
        CLEAR_STRUCT(AddEffectRack);
        CLEAR_STRUCT(RemoveEffectRack);
        CLEAR_STRUCT(AddChainToRack);
        CLEAR_STRUCT(RemoveChainFromRack);
        CLEAR_STRUCT(EnableInputChannelForChain);
        CLEAR_STRUCT(DisableInputChannelForChain);
        CLEAR_STRUCT(AddEffectToChain);
        CLEAR_STRUCT(RemoveEffectFromChain);
        CLEAR_STRUCT(SetEffectChainParameters);
        CLEAR_STRUCT(SetEffectParameters);
        CLEAR_STRUCT(SetParameterParameters);
#undef CLEAR_STRUCT
    }

    // This is called from the main thread by EffectsManager after receiving a
    // response from EngineEffectsManager in the audio engine thread.
    ~EffectsRequest() {
        if (type == ENABLE_EFFECT_CHAIN_FOR_INPUT_CHANNEL) {
            VERIFY_OR_DEBUG_ASSERT(EnableInputChannelForChain.pEffectStatesMapArray != nullptr) {
                return;
            }
            // This only deletes the container used to passed the EffectStates
            // to EffectProcessorImpl. The EffectStates are managed by
            // EffectProcessorImpl.
            delete EnableInputChannelForChain.pEffectStatesMapArray;
        }
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
        // - ENABLE_EFFECT_CHAIN_FOR_INPUT_CHANNEL
        // - DISABLE_EFFECT_CHAIN_FOR_INPUT_CHANNEL
        EngineEffectChain* pTargetChain;
        // Used by:
        // - SET_EFFECT_PARAMETER
        EngineEffect* pTargetEffect;
    };

    // Message-specific data.
    union {
        struct {
            EngineEffectRack* pRack;
            SignalProcessingStage signalProcessingStage;
        } AddEffectRack;
        struct {
            EngineEffectRack* pRack;
            SignalProcessingStage signalProcessingStage;
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
            EffectStatesMapArray* pEffectStatesMapArray;
            const ChannelHandle* pChannelHandle;
        } EnableInputChannelForChain;
        struct {
            const ChannelHandle* pChannelHandle;
        } DisableInputChannelForChain;
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
            EffectChainMixMode mix_mode;
            double mix;
        } SetEffectChainParameters;
        struct {
            bool enabled;
        } SetEffectParameters;
        struct {
            int iParameter;
        } SetParameterParameters;
    };

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
        EffectsRequest& message,
        EffectsResponsePipe* pResponsePipe) = 0;
};
