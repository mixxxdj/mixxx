#pragma once

#include <QList>
#include <QSet>
#include <QString>

#include "effects/defs.h"

class EffectProcessor;

/// EffectsBackend is an abstract base class that enumerates available effects
/// which are identified by EffectManifests. EffectsBackends create an
/// EffectProcessor when provided with an EffectManifest indicating which
/// specific EffectProcessor type to create. EffectProcessors implement the DSP
/// logic specific to each effect.
///
/// Currently the implemented EffectsBackend subclasses are for the effects
/// built into Mixxx and LV2 plugins. Other plugin types such as VSTs could be
/// added in the future by creating new subclasses of EffectsBackend,
/// EffectManifest, EffectState, and EffectProcessorImpl.
class EffectsBackend {
  public:
    virtual ~EffectsBackend(){};

    virtual EffectBackendType getType() const = 0;

    /// returns a list sorted like it should be displayed in the GUI
    virtual const QList<QString> getEffectIds() const = 0;
    /// returns a pointer to the manifest or a null pointer in case a
    /// previously stored effect is no longer available
    virtual EffectManifestPointer getManifest(const QString& effectId) const = 0;
    virtual const QList<EffectManifestPointer> getManifests() const = 0;
    virtual bool canInstantiateEffect(const QString& effectId) const = 0;

    virtual std::unique_ptr<EffectProcessor> createProcessor(
            const EffectManifestPointer pManifest) const = 0;

    static EffectBackendType backendTypeFromString(const QString& typeName);
    static QString backendTypeToString(EffectBackendType backendType);
    /// Use this when showing the string in the GUI
    static QString translatedBackendName(EffectBackendType backendType);
};
