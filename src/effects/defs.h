#pragma once

#include <QSharedPointer>
#include <array>
#include <memory>

#include "engine/channelhandle.h"
#include "util/compatibility/qhash.h"

enum class EffectEnableState {
    Disabled,
    Enabled,
    Disabling,
    Enabling
};

// The order of the enum values here is used to determine sort order in EffectManifest::alphabetize
enum class EffectBackendType {
    BuiltIn,
    AudioUnit,
    LV2,
    Unknown
};

inline qhash_seed_t qHash(const EffectBackendType& backendType) {
    return static_cast<qhash_seed_t>(backendType);
}

enum class SignalProcessingStage {
    Prefader,
    Postfader
};

inline qhash_seed_t qHash(
        SignalProcessingStage stage,
        qhash_seed_t seed = 0) {
    return qHash(static_cast<uint>(stage), seed);
};

constexpr int kNumStandardEffectUnits = 4;
constexpr int kNumEffectsPerUnit = 4;

const QString kNoEffectString = QStringLiteral("---");

const QString kMixerProfile = QStringLiteral("[Mixer Profile]");
const QString kHighEqFrequency = QStringLiteral("HiEQFrequency");
const QString kLowEqFrequency = QStringLiteral("LoEQFrequency");

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

class EffectChain;
typedef QSharedPointer<EffectChain> EffectChainPointer;

class EffectChainPreset;
typedef QSharedPointer<EffectChainPreset> EffectChainPresetPointer;

class EffectPreset;
typedef QSharedPointer<EffectPreset> EffectPresetPointer;

class StandardEffectChain;
typedef QSharedPointer<StandardEffectChain> StandardEffectChainPointer;

class EqualizerEffectChain;
typedef QSharedPointer<EqualizerEffectChain> EqualizerEffectChainPointer;

class OutputEffectChain;
typedef QSharedPointer<OutputEffectChain> OutputEffectChainPointer;

class QuickEffectChain;
typedef QSharedPointer<QuickEffectChain> QuickEffectChainPointer;
