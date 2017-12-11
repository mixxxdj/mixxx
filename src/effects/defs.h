#pragma once
#include "engine/channelhandle.h"

enum class EffectEnableState {
    Disabled,
    Enabled,
    Disabling,
    Enabling
};

class EffectState;
// For sending EffectStates along the MessagePipe
typedef ChannelHandleMap<EffectState*>* EffectStatesPointer;
