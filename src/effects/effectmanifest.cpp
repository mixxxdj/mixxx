#include "effects/effectmanifest.h"

QDebug operator<<(QDebug dbg, const EffectManifest& manifest) {
    return dbg.maybeSpace() << QString("EffectManifest(%1)").arg(manifest.id());
}

bool EffectManifest::hasMetaKnobLinking() const {
    for (const auto& pParameterManifest : m_parameters) {
        if (pParameterManifest->defaultLinkType() !=
                EffectManifestParameter::LinkType::NONE) {
            return true;
        }
    }
    return false;
}

bool EffectManifest::alphabetize(
        EffectManifestPointer pManifest1, EffectManifestPointer pManifest2) {
    // Sort built-in effects first before external plugins
    int backendNameComparision = static_cast<int>(pManifest1->backendType()) -
            static_cast<int>(pManifest2->backendType());
    int displayNameComparision = QString::localeAwareCompare(
            pManifest1->displayName(), pManifest2->displayName());
    return (backendNameComparision ? (backendNameComparision < 0) : (displayNameComparision < 0));
}
