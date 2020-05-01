#pragma once

#include "effects/effectsbackend.h"

class ControlObject;
class EffectProcessor;

/// EffectsBackendManager initializes EffectsBackends, maintains the list of
/// available EffectManifests, and creates EffectProcessors from EffectManifests.
class EffectsBackendManager {
  public:
    EffectsBackendManager();
    ~EffectsBackendManager() = default;

    inline const QList<EffectManifestPointer>& getManifests() const {
        return m_manifests;
    };
    EffectManifestPointer getManifestFromUniqueId(const QString& uid) const;
    EffectManifestPointer getManifest(const QString& id, EffectBackendType backendType) const;
    const QString getDisplayNameForEffectPreset(EffectPresetPointer pPreset) const;
    bool isEQ(const QString& effectId) const;

    std::unique_ptr<EffectProcessor> createProcessor(const EffectManifestPointer pManifest);

  private:
    void addBackend(EffectsBackendPointer pEffectsBackend);

    std::unique_ptr<ControlObject> m_pNumEffectsAvailable;

    QHash<EffectBackendType, EffectsBackendPointer> m_effectsBackends;
    QList<EffectManifestPointer> m_manifests;
};

typedef QSharedPointer<EffectsBackendManager> EffectsBackendManagerPointer;
