#include "effects/backends/au/aubackend.h"

#include <memory>

#include "effects/backends/au/aubackend.h"
#include "effects/defs.h"

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
