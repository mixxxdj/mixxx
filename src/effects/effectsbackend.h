#pragma once

#include <QObject>
#include <QList>
#include <QSet>
#include <QString>

#include "effects/defs.h"
#include "effects/effectslot.h"
#include "preferences/usersettings.h"

#include "util/memory.h"

class EffectProcessor;

// EffectsBackend is an abstract base class that enumerates available effects
// which are identified by EffectManifests. EffectsBackend creates
// EffectProcessors when provided with an EffectManifest from EffectsManager
// indicating which specific EffectProcessor type to create.

// The EffectProcessors implement the DSP logic specific to each effect.
// EffectManager sends the EffectProcessors down to the EffectChainSlot, which
// sends it down to the EffectSlot. The EffectSlot uses the EffectProcessor to
// create an EngineEffect and add/remove the EngineEffect from the engine.

// Currently the implemented EffectsBackend subclasses are for the effects
// built into Mixxx and LV2 plugins. Other plugin types such as VSTs could be
// added in the future by creating new subclasses of EffectsBackend,
// EffectManifest, EffectState, and EffectProcessorImpl.
class EffectsBackend {
  public:
    virtual ~EffectsBackend() {};

    virtual EffectBackendType getType() const = 0;

    // returns a list sorted like it should be displayed in the GUI
    virtual const QList<QString> getEffectIds() const = 0;
    virtual EffectManifestPointer getManifest(const QString& effectId) const = 0;
    virtual const QList<EffectManifestPointer> getManifests() const = 0;
    virtual bool canInstantiateEffect(const QString& effectId) const = 0;

    virtual std::unique_ptr<EffectProcessor> createProcessor(
        const EffectManifestPointer pManifest) const = 0;

    static EffectBackendType backendTypeFromString(const QString& string) {
        if (string == "LV2") {
            return EffectBackendType::LV2;
        } else {
            return EffectBackendType::BuiltIn;
        }
    }

    static QString backendTypeToString(EffectBackendType backendType) {
        if (backendType == EffectBackendType::BuiltIn) {
            return "BuiltIn";
        } else if (backendType == EffectBackendType::LV2) {
            return "LV2";
        } else {
            return "Unknown";
        }
    }
};
