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
