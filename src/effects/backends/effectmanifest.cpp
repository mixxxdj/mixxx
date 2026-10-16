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
    // Defensive: null manifests should no longer occur here, but
    // this check is retained to prevent std::sort from crashing if an
    // unexpected null pointer ever reaches this comparator. Nulls sort to the end.
    VERIFY_OR_DEBUG_ASSERT(pManifest1) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(pManifest2) {
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
