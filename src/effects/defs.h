#pragma once
#include "util/memory.h"
#include "engine/channelhandle.h"
#include <array>

enum class EffectEnableState {
    Disabled,
    Enabled,
    Disabling,
    Enabling
};

enum class SignalProcessingStage {
    Prefader,
    Postfader
};

inline uint qHash(SignalProcessingStage stage) {
    return static_cast<uint>(stage);
};

enum class EffectChainInsertionType {
    Insert = 0,
    Send,
    // The number of insertion types. Also used to represent "unknown".
    Num_Insertion_Types
};

constexpr int kNumEffectsPerUnit = 4;

// NOTE: Setting this to true will enable string manipulation and calls to
// qDebug() in the audio engine thread. That may cause audio dropouts, so only
// enable this when debugging the effects system.
constexpr bool kEffectDebugOutput = false;

class EffectState;
// For sending EffectStates along the MessagePipe
typedef ChannelHandleMap<EffectState*> EffectStatesMap;
typedef std::array<EffectStatesMap, kNumEffectsPerUnit> EffectStatesMapArray;
