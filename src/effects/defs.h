#pragma once
#include "util/memory.h"
#include "engine/channelhandle.h"

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

const int kNumEffectsPerUnit = 4;

class EffectState;
// For sending EffectStates along the MessagePipe
typedef ChannelHandleMap<EffectState*> EffectStatesMap;
typedef std::array<EffectStatesMap, kNumEffectsPerUnit> EffectStatesMapArray;
