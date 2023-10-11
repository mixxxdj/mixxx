#include "effects/backends/au/aubackend.h"

#include <memory>

#include "effects/backends/au/aubackend.h"
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

EffectsBackendPointer createAUBackend() {
    return EffectsBackendPointer(new AUBackend());
}

AUBackend::AUBackend() {
}

AUBackend::~AUBackend() {
}

const QList<QString> AUBackend::getEffectIds() const {
    // TODO
    return {};
}

EffectManifestPointer AUBackend::getManifest(const QString& effectId) const {
    // TODO
    return {};
}

const QList<EffectManifestPointer> AUBackend::getManifests() const {
    // TODO
    return {};
}

bool AUBackend::canInstantiateEffect(const QString& effectId) const {
    // TODO
    return false;
}

std::unique_ptr<EffectProcessor> AUBackend::createProcessor(
        const EffectManifestPointer pManifest) const {
    // TODO
    return {};
}
