#pragma once

#include <QString>
#include <QVariant>
#include <QtGlobal>

#include "effects/defs.h"
#include "effects/effectchainmixmode.h"
#include "engine/channelhandle.h"
#include "util/memory.h"
#include "util/messagepipe.h"

class EngineEffectChain;
class EngineEffect;

struct EffectsRequest {
    enum MessageType {
        // Messages for EngineEffectChain
        ADD_EFFECT_CHAIN,
        REMOVE_EFFECT_CHAIN,
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

    // Creates a new uninitialized EffectsRequest.  Callers are responsible for making sure that
    // they initialize all the values of the struct corresponding to the type they select.
    EffectsRequest()
            : type(NUM_REQUEST_TYPES),
              request_id(-1),
              value(0.0) {
        pTargetChain = nullptr;
        pTargetEffect = nullptr;
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
            EngineEffectChain* pChain;
            SignalProcessingStage signalProcessingStage;
        } AddEffectChain;
        struct {
            EngineEffectChain* pChain;
            SignalProcessingStage signalProcessingStage;
        } RemoveEffectChain;
        struct {
            EffectStatesMapArray* pEffectStatesMapArray;
            ChannelHandle channelHandle;
        } EnableInputChannelForChain;
        struct {
            ChannelHandle channelHandle;
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
            EffectChainMixMode::Type mix_mode;
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
    double value;
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

    EffectsResponse(const EffectsRequest& request, bool succeeded = false)
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
    virtual ~EffectsRequestHandler() = default;
};
