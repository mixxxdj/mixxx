#pragma once

#include <lilv/lilv.h>

#include "effects/backends/effectsbackend.h"
#include "effects/backends/lv2/lv2manifest.h"
#include "effects/defs.h"

/// Refer to EffectsBackend for documentation
class LV2Backend : public EffectsBackend {
  public:
    LV2Backend();
    virtual ~LV2Backend();

    EffectBackendType getType() const {
        return EffectBackendType::LV2;
    };

    const QList<QString> getEffectIds() const;
    const QSet<QString> getDiscoveredPluginIds() const;
    EffectManifestPointer getManifest(const QString& effectId) const;
    const QList<EffectManifestPointer> getManifests() const;
    LV2EffectManifestPointer getLV2Manifest(const QString& effectId) const;
    std::unique_ptr<EffectProcessor> createProcessor(
            const EffectManifestPointer pManifest) const;
    bool canInstantiateEffect(const QString& effectId) const;

  private:
    void enumeratePlugins();
    void initializeProperties();
    LilvWorld* m_pWorld;
    QHash<QString, LilvNode*> m_properties;
    QHash<QString, LV2EffectManifestPointer> m_registeredEffects;

    QString debugString() const {
        return "LV2Backend";
    }
};
