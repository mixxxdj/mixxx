#include "preferences/effectprofile.h"

EffectProfile::EffectProfile(EffectManifestPointer pManifest,
                             bool visibility,
                             QObject* parent)
    : QObject(parent),
      m_pManifest(pManifest),
      m_isVisible(visibility) {
}

QString EffectProfile::getEffectId() const {
    return m_pManifest->id();
}

QString EffectProfile::getDisplayName() const {
    return m_pManifest->displayName();
}

bool EffectProfile::isVisible() const {
    return m_isVisible;
}

void EffectProfile::setVisibility(bool value) {
    m_isVisible = value;
}

EffectManifestPointer EffectProfile::getManifest() const {
    return m_pManifest;
}