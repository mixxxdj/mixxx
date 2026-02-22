#include "effects/presets/effectpresetmanager.h"

#include <QDir>

#include "effects/backends/effectsbackendmanager.h"
#include "effects/presets/effectpreset.h"
#include "effects/presets/effectxmlelements.h"
#include "util/filename.h"

namespace {
const QString kEffectDefaultsDirectory = "/effects/defaults";
} // namespace

EffectPresetManager::EffectPresetManager(UserSettingsPointer pConfig,
        EffectsBackendManagerPointer pBackendManager)
        : m_pConfig(pConfig),
          m_pBackendManager(pBackendManager) {
    loadDefaultEffectPresets();
}

EffectPresetManager::~EffectPresetManager() {
    for (const auto& pEffectPreset : std::as_const(m_defaultPresets)) {
        saveDefaultForEffect(pEffectPreset);
    }
}

void EffectPresetManager::loadDefaultEffectPresets() {
    // Load saved defaults from settings directory
    QString dirPath(m_pConfig->getSettingsPath() + kEffectDefaultsDirectory);
    QDir effectsDefaultsDir(dirPath);
    effectsDefaultsDir.setFilter(QDir::Files | QDir::Readable);
    const auto& fileNames = effectsDefaultsDir.entryList();
    for (const auto& filePath : fileNames) {
        QFile file(dirPath + "/" + filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }
        QDomDocument doc;
        if (!doc.setContent(&file)) {
            file.close();
            continue;
        }
        auto presetFromFile = EffectPreset(doc.documentElement());
        if (!presetFromFile.isEmpty()) {
            EffectManifestPointer pManifest = m_pBackendManager->getManifest(
                    presetFromFile.id(), presetFromFile.backendType());
            if (pManifest) {
                auto pEffectPreset = EffectPresetPointer(new EffectPreset(pManifest));
                pEffectPreset->updateParametersFrom(presetFromFile);
                m_defaultPresets.insert(pManifest, pEffectPreset);
            }
        }
        file.close();
    }

    // If no preset was found, generate one from the manifest
    for (const auto& pManifest : m_pBackendManager->getManifests()) {
        if (!m_defaultPresets.contains(pManifest)) {
            m_defaultPresets.insert(pManifest,
                    EffectPresetPointer(new EffectPreset(pManifest)));
        }
    }
}

void EffectPresetManager::saveDefaultForEffect(EffectSlotPointer pEffectSlot) {
    EffectPresetPointer pPreset(new EffectPreset(pEffectSlot));
    saveDefaultForEffect(pPreset);
}

void EffectPresetManager::saveDefaultForEffect(EffectPresetPointer pEffectPreset) {
    if (pEffectPreset->isEmpty()) {
        return;
    }

    const auto pManifest = m_pBackendManager->getManifest(
            pEffectPreset->id(), pEffectPreset->backendType());
    VERIFY_OR_DEBUG_ASSERT(pManifest) {
        return;
    }
    m_defaultPresets.insert(pManifest, pEffectPreset);

    QDomDocument doc(EffectXml::kEffect);
    doc.setContent(EffectXml::kFileHeader);
    doc.appendChild(pEffectPreset->toXml(&doc));

    QString path(m_pConfig->getSettingsPath() + kEffectDefaultsDirectory);
    QDir effectsDefaultsDir(path);
    if (!effectsDefaultsDir.exists()) {
        effectsDefaultsDir.mkpath(path);
    }

    // The file name does not matter as long as it is unique. The actual id string
    // is safely stored in the UTF8 document, regardless of what the filesystem
    // supports for file names.
    QFile file(path + "/" + mixxx::filename::sanitize(pEffectPreset->id()) + ".xml");
    if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        return;
    }
    file.write(doc.toString().toUtf8());
    file.close();
}
