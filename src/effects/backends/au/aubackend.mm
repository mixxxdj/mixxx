#include "effects/backends/au/aubackend.h"

#include <memory>

#include "effects/backends/au/aubackend.h"
#include "effects/backends/au/aueffectprocessor.h"
#include "effects/defs.h"

/// An effects backend for Audio Unit (AU) plugins. macOS-only.
class AUBackend : public EffectsBackend {
  public:
    AUBackend() {
    }

    ~AUBackend() override {
    }

    EffectBackendType getType() const override {
        return EffectBackendType::AU;
    };

    const QList<QString> getEffectIds() const override {
        // TODO
        return {};
    }

    EffectManifestPointer getManifest(const QString& effectId) const override {
        // TODO
        return {};
    }

    const QList<EffectManifestPointer> getManifests() const override {
        // TODO
        return {};
    }

    bool canInstantiateEffect(const QString& effectId) const override {
        // TODO
        return false;
    }

    std::unique_ptr<EffectProcessor> createProcessor(
            const EffectManifestPointer pManifest) const override {
        return std::make_unique<AUEffectProcessor>(pManifest);
    }
};

EffectsBackendPointer createAUBackend() {
    return EffectsBackendPointer(new AUBackend());
}
