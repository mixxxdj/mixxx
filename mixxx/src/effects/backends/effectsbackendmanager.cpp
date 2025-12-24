#include "effects/backends/effectsbackendmanager.h"

#include "control/controlobject.h"
#include "effects/backends/builtin/builtinbackend.h"
#include "effects/backends/effectmanifest.h"
#include "effects/backends/effectprocessor.h"
#ifdef __AU_EFFECTS__
#include "effects/backends/audiounit/audiounitbackend.h"
#endif
#ifdef __LILV__
#include "effects/backends/lv2/lv2backend.h"
#endif
#include "effects/presets/effectpreset.h"

EffectsBackendManager::EffectsBackendManager() {
    m_pNumEffectsAvailable = std::make_unique<ControlObject>(
            ConfigKey("[Master]", "num_effectsavailable"));
    m_pNumEffectsAvailable->setReadOnly();

    addBackend(EffectsBackendPointer(new BuiltInBackend()));
#ifdef __AU_EFFECTS__
    addBackend(createAudioUnitBackend());
#endif
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
            EffectManifest::sortLexigraphically);
}

const QList<EffectManifestPointer> EffectsBackendManager::getManifestsForBackend(
        EffectBackendType backendType) const {
    auto pBackend = m_effectsBackends.value(backendType);
    VERIFY_OR_DEBUG_ASSERT(pBackend) {
        return QList<EffectManifestPointer>();
    }
    auto list = pBackend->getManifests();
    std::sort(list.begin(), list.end(), EffectManifest::sortLexigraphically);
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
    auto backendType = EffectBackendType::BuiltIn;
    // Mixxx 2.0 - 2.3 did not store the backend type in mixxx.cfg,
    // so this code will be executed once when upgrading to Mixxx 2.4.
    // If it is triggered at any later time, there is a bug somewhere.
    // Do not manipulate the string passed to this function, just pass
    // it directly to BuiltInBackend.
    if (delimiterIndex == -1) {
        auto pEffectsBackend = m_effectsBackends.value(EffectBackendType::BuiltIn);
        VERIFY_OR_DEBUG_ASSERT(pEffectsBackend) {
            return {};
        }
        return pEffectsBackend->getManifest(uid);
    }
    backendType = EffectsBackend::backendTypeFromString(uid.mid(delimiterIndex + 1));
    auto pEffectsBackend = m_effectsBackends.value(backendType);
    VERIFY_OR_DEBUG_ASSERT(pEffectsBackend) {
        return {};
    }
    return pEffectsBackend->getManifest(uid.mid(-1, delimiterIndex + 1));
}

EffectManifestPointer EffectsBackendManager::getManifest(
        const QString& id, EffectBackendType backendType) const {
    auto pEffectsBackend = m_effectsBackends.value(backendType);
    VERIFY_OR_DEBUG_ASSERT(pEffectsBackend) {
        return {};
    }
    return pEffectsBackend->getManifest(id);
}

EffectManifestPointer EffectsBackendManager::getManifest(EffectPresetPointer pPreset) const {
    EffectManifestPointer pManifest = getManifest(pPreset->id(), pPreset->backendType());
    if (!pManifest) {
        qWarning() << "Failed to find manifest for effect preset " << pPreset->id();
    }
    return pManifest;
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
