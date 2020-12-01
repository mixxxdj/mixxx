#include "effects/backends/effectsbackendmanager.h"

#include "control/controlobject.h"
#include "effects/backends/builtin/builtinbackend.h"
#include "effects/backends/effectprocessor.h"
#ifdef __LILV__
#include "effects/backends/lv2/lv2backend.h"
#endif
#include "effects/presets/effectpreset.h"

EffectsBackendManager::EffectsBackendManager() {
    m_pNumEffectsAvailable = std::make_unique<ControlObject>(
            ConfigKey("[Master]", "num_effectsavailable"));
    m_pNumEffectsAvailable->setReadOnly();

    addBackend(EffectsBackendPointer(new BuiltInBackend()));
#ifdef __LILV__
    addBackend(EffectsBackendPointer(new LV2Backend()));
#endif
}

void EffectsBackendManager::addBackend(EffectsBackendPointer pBackend) {
    VERIFY_OR_DEBUG_ASSERT(pBackend) {
        return;
    }

    m_effectsBackends.insert(pBackend->getType(), pBackend);

    for (const QString& effectId : pBackend->getEffectIds()) {
        m_manifests.append(pBackend->getManifest(effectId));
    }

    m_pNumEffectsAvailable->forceSet(m_manifests.size());

    std::sort(m_manifests.begin(),
            m_manifests.end(),
            EffectManifest::alphabetize);
}

const QList<EffectManifestPointer> EffectsBackendManager::getManifestsForBackend(
        EffectBackendType backendType) const {
    auto pBackend = m_effectsBackends.value(backendType);
    VERIFY_OR_DEBUG_ASSERT(pBackend) {
        return QList<EffectManifestPointer>();
    }
    auto list = pBackend->getManifests();
    std::sort(list.begin(), list.end(), EffectManifest::alphabetize);
    return list;
}

EffectManifestPointer EffectsBackendManager::getManifestFromUniqueId(
        const QString& uid) const {
    if (kEffectDebugOutput) {
        //qDebug() << "EffectsBackendManager::getManifestFromUniqueId" << uid;
    }
    if (uid.isEmpty()) {
        // Do not DEBUG_ASSERT, this may be a valid request for a nullptr to
        // unload an effect.
        return EffectManifestPointer();
    }
    int delimiterIndex = uid.lastIndexOf(" ");
    EffectBackendType backendType =
            EffectManifest::backendTypeFromString(uid.mid(delimiterIndex + 1));
    VERIFY_OR_DEBUG_ASSERT(backendType != EffectBackendType::Unknown) {
        // Mixxx 2.0 - 2.2 did not store the backend type in mixxx.cfg,
        // so this code will be executed once when upgrading to Mixxx 2.3.
        // This debug assertion is safe to ignore in that case. If it is
        // triggered at any later time, there is a bug somewhere.
        // Do not manipulate the string passed to this function, just pass
        // it directly to BuiltInBackend.
        return m_effectsBackends.value(EffectBackendType::BuiltIn)
                ->getManifest(uid);
    }
    return m_effectsBackends.value(backendType)
            ->getManifest(uid.mid(-1, delimiterIndex + 1));
}

EffectManifestPointer EffectsBackendManager::getManifest(
        const QString& id, EffectBackendType backendType) const {
    return m_effectsBackends.value(backendType)->getManifest(id);
}

const QString EffectsBackendManager::getDisplayNameForEffectPreset(
        EffectPresetPointer pPreset) const {
    //: Displayed when no effect is loaded
    QString displayName(QObject::tr("None"));
    if (!pPreset || pPreset->isEmpty()) {
        return displayName;
    }

    bool manifestFound = false;
    for (const auto& pManifest : std::as_const(m_manifests)) {
        if (pManifest->id() == pPreset->id() &&
                pManifest->backendType() == pPreset->backendType()) {
            displayName = pManifest->name();
            manifestFound = true;
            break;
        }
    }
    DEBUG_ASSERT(manifestFound);
    return displayName;
}

std::unique_ptr<EffectProcessor> EffectsBackendManager::createProcessor(
        const EffectManifestPointer pManifest) {
    if (!pManifest) {
        // This can be a valid request to unload an effect, so do not DEBUG_ASSERT.
        return std::unique_ptr<EffectProcessor>(nullptr);
    }
    EffectsBackendPointer pBackend = m_effectsBackends.value(pManifest->backendType());
    VERIFY_OR_DEBUG_ASSERT(pBackend) {
        return std::unique_ptr<EffectProcessor>(nullptr);
    }
    return pBackend->createProcessor(pManifest);
}
