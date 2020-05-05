#pragma once
#include "util/memory.h"
#include "engine/channelhandle.h"
#include <array>
#include <QSharedPointer>
#include <memory>

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

inline uint qHash(const EffectBackendType& backendType) {
    return static_cast<uint>(backendType);
}

enum class SignalProcessingStage {
    Prefader,
    Postfader
};

inline uint qHash(SignalProcessingStage stage) {
    return static_cast<uint>(stage);
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

class EffectsBackend;
typedef QSharedPointer<EffectsBackend> EffectsBackendPointer;

class EffectsMessenger;
typedef QSharedPointer<EffectsMessenger> EffectsMessengerPointer;

class VisibleEffectsList;
typedef QSharedPointer<VisibleEffectsList> VisibleEffectsListPointer;

class EffectsBackendManager;
typedef QSharedPointer<EffectsBackendManager> EffectsBackendManagerPointer;

class EffectPresetManager;
typedef QSharedPointer<EffectPresetManager> EffectPresetManagerPointer;

class EffectChainPresetManager;
typedef QSharedPointer<EffectChainPresetManager> EffectChainPresetManagerPointer;

class EffectState;
// For sending EffectStates along the MessagePipe
typedef ChannelHandleMap<EffectState*> EffectStatesMap;
typedef std::array<EffectStatesMap, kNumEffectsPerUnit> EffectStatesMapArray;

class EngineEffectParameter;
typedef QSharedPointer<EngineEffectParameter> EngineEffectParameterPointer;

class EffectParameter;
typedef QSharedPointer<EffectParameter> EffectParameterPointer;

class EffectSlot;
typedef QSharedPointer<EffectSlot> EffectSlotPointer;

class EffectKnobParameterSlot;
typedef QSharedPointer<EffectKnobParameterSlot> EffectKnobParameterSlotPointer;

class EffectButtonParameterSlot;
typedef QSharedPointer<EffectButtonParameterSlot> EffectButtonParameterSlotPointer;

class EffectManifest;
typedef QSharedPointer<EffectManifest> EffectManifestPointer;

class EffectParameterSlotBase;
typedef QSharedPointer<EffectParameterSlotBase> EffectParameterSlotBasePointer;

class EffectChainSlot;
typedef QSharedPointer<EffectChainSlot> EffectChainSlotPointer;

class EffectChainPreset;
typedef QSharedPointer<EffectChainPreset> EffectChainPresetPointer;

class EffectPreset;
typedef QSharedPointer<EffectPreset> EffectPresetPointer;

class StandardEffectChainSlot;
typedef QSharedPointer<StandardEffectChainSlot> StandardEffectChainSlotPointer;

class EqualizerEffectChainSlot;
typedef QSharedPointer<EqualizerEffectChainSlot> EqualizerEffectChainSlotPointer;

class OutputEffectChainSlot;
typedef QSharedPointer<OutputEffectChainSlot> OutputEffectChainSlotPointer;

class QuickEffectChainSlot;
typedef QSharedPointer<QuickEffectChainSlot> QuickEffectChainSlotPointer;
