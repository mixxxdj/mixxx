#include "effects/backends/effectmanifest.h"

QDebug operator<<(QDebug dbg, const EffectManifest& manifest) {
    return dbg.maybeSpace() << QString("EffectManifest(%1)").arg(manifest.id());
}

bool EffectManifest::hasMetaKnobLinking() const {
    for (const auto& pParameterManifest : m_parameters) {
        if (pParameterManifest->defaultLinkType() !=
                EffectManifestParameter::LinkType::None) {
            return true;
        }
    }
    return false;
}

bool EffectManifest::sortLexigraphically(
        EffectManifestPointer pManifest1, EffectManifestPointer pManifest2) {
    // Defensive: null manifests sort to the end. A backend whose async
    // manifest loading times out may briefly expose null entries; crashing
    // std::sort on them is worse than a transiently misordered list.
    if (!pManifest1) {
        return false;
    }
    if (!pManifest2) {
        return true;
    }
    // Sort built-in effects first before external plugins
    int backendNameComparision = static_cast<int>(pManifest1->backendType()) -
            static_cast<int>(pManifest2->backendType());
    if (backendNameComparision != 0) {
        return backendNameComparision < 0;
    }

    int displayNameComparision = QString::localeAwareCompare(
            pManifest1->displayName(), pManifest2->displayName());
    return displayNameComparision < 0;
}
