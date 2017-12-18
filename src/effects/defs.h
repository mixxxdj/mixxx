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

const int kNumEffectsPerUnit = 4;

class EffectState;
// For sending EffectStates along the MessagePipe
typedef ChannelHandleMap<EffectState*> EffectStatesMap;
typedef std::array<EffectStatesMap, kNumEffectsPerUnit> EffectStatesMapArray;
