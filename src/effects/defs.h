#pragma once
#include "util/memory.h"
#include "engine/channelhandle.h"
#include <array>
#include <QSharedPointer>

enum class EffectEnableState {
    Disabled,
    Enabled,
    Disabling,
    Enabling
};

enum class EffectBackendType {
    BuiltIn,
    LV2,
    Unknown
};

enum class SignalProcessingStage {
    Prefader,
    Postfader
};

inline uint qHash(
        SignalProcessingStage stage,
        uint seed = 0) {
    return qHash(static_cast<uint>(stage), seed);
};

enum class EffectChainMixMode {
    DrySlashWet = 0, // Crossfade between dry and wet
    DryPlusWet, // Add wet to dry
    NumMixModes // The number of mix modes. Also used to represent "unknown".
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

class EffectRack;
typedef QSharedPointer<EffectRack> EffectRackPointer;

class StandardEffectRack;
typedef QSharedPointer<StandardEffectRack> StandardEffectRackPointer;

class EqualizerRack;
typedef QSharedPointer<EqualizerRack> EqualizerRackPointer;

class QuickEffectRack;
typedef QSharedPointer<QuickEffectRack> QuickEffectRackPointer;

class OutputEffectRack;
typedef QSharedPointer<OutputEffectRack> OutputEffectRackPointer;

class EffectSlot;
typedef QSharedPointer<EffectSlot> EffectSlotPointer;

class EffectParameterSlot;
typedef QSharedPointer<EffectParameterSlot> EffectParameterSlotPointer;

class EffectButtonParameterSlot;
typedef QSharedPointer<EffectButtonParameterSlot> EffectButtonParameterSlotPointer;

class EffectManifest;
typedef QSharedPointer<EffectManifest> EffectManifestPointer;

class Effect;
typedef QSharedPointer<Effect> EffectPointer;

class EffectParameterSlotBase;
typedef QSharedPointer<EffectParameterSlotBase> EffectParameterSlotBasePointer;

class EffectChainSlot;
typedef QSharedPointer<EffectChainSlot> EffectChainSlotPointer;
