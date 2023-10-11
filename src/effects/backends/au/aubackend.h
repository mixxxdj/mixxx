#pragma once

#include <QList>
#include <QString>

#include "effects/backends/effectprocessor.h"
#include "effects/backends/effectsbackend.h"
#include "effects/defs.h"

/// An effects backend for Audio Unit (AU) plugins. macOS-only.
class AUBackend : public EffectsBackend {
  public:
    AUBackend();
    ~AUBackend() override;

    EffectBackendType getType() const override {
        return EffectBackendType::AU;
    };

    const QList<QString> getEffectIds() const override;
    EffectManifestPointer getManifest(const QString& effectId) const override;
    const QList<EffectManifestPointer> getManifests() const override;
    bool canInstantiateEffect(const QString& effectId) const override;
    std::unique_ptr<EffectProcessor> createProcessor(
            const EffectManifestPointer pManifest) const override;
};
