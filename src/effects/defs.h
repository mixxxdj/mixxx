#pragma once
#include "util/memory.h"
#include "engine/channelhandle.h"

enum class EffectEnableState {
    Disabled,
    Enabled,
    Disabling,
    Enabling
};

const int kNumEffectsPerUnit = 4;

class EffectState;
// For sending EffectStates along the MessagePipe
typedef ChannelHandleMap<EffectState*> EffectStatesMap;
typedef std::array<std::unique_ptr<EffectStatesMap>, kNumEffectsPerUnit> EffectStatesMapArray;
