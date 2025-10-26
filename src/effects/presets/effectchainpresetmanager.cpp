#include "effects/presets/effectchainpresetmanager.h"

#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

#include "effects/backends/builtin/biquadfullkilleqeffect.h"
#include "effects/backends/builtin/filtereffect.h"
#include "effects/backends/effectmanifest.h"
#include "effects/effectchain.h"
#include "effects/presets/effectchainpreset.h"
#include "effects/presets/effectpreset.h"
#include "effects/presets/effectxmlelements.h"
#include "moc_effectchainpresetmanager.cpp"
#include "util/filename.h"
#include "util/xml.h"

namespace {
const QString kEffectChainPresetDirectory = QStringLiteral("/effects/chains");
const QString kXmlFileExtension = QStringLiteral(".xml");
const QString kFolderDelimiter = QStringLiteral("/");

EffectChainPresetPointer loadPresetFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open chain preset file" << filePath;
        return nullptr;
    }
    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        qWarning() << "Could not read XML from chain preset file" << filePath;
        return nullptr;
    }
    auto pEffectChainPreset = EffectChainPresetPointer::create(doc.documentElement());
    file.close();
    return pEffectChainPreset;
}

EffectChainPresetPointer createEmptyReadOnlyChainPreset() {
    EffectManifestPointer pEmptyManifest(new EffectManifest());
    pEmptyManifest->setName(kNoEffectString);
    // Center the Super knob, eliminates the colored (bipolar) knob ring
    pEmptyManifest->setMetaknobDefault(0.5);
    // Required for the QuickEffect selector in DlgPrefEQ
    pEmptyManifest->setShortName(kNoEffectString);
    auto pEmptyChainPreset = EffectChainPresetPointer::create(pEmptyManifest);
    pEmptyChainPreset->setReadOnly();

    return pEmptyChainPreset;
}

} // anonymous namespace

EffectChainPresetManager::EffectChainPresetManager(UserSettingsPointer pConfig,
        EffectsBackendManagerPointer pBackendManager)
        : m_pConfig(pConfig),
          m_pBackendManager(pBackendManager) {
}

int EffectChainPresetManager::presetIndex(const QString& presetName) const {
    if (m_effectChainPresets.contains(presetName)) {
        EffectChainPresetPointer pPreset =
                m_effectChainPresets.value(presetName);
        return m_effectChainPresetsSorted.indexOf(pPreset);
    }
    return -1;
}

int EffectChainPresetManager::presetIndex(
        EffectChainPresetPointer pPreset) const {
    return m_effectChainPresetsSorted.indexOf(pPreset);
}

EffectChainPresetPointer EffectChainPresetManager::presetAtIndex(
        int index) const {
    if (index < 0) {
        index = m_effectChainPresetsSorted.size() - 1;
    } else if (index >= m_effectChainPresetsSorted.size()) {
        index = 0;
    }
    return m_effectChainPresetsSorted.at(index);
}

int EffectChainPresetManager::quickEffectPresetIndex(const QString& presetName) const {
    if (m_effectChainPresets.contains(presetName)) {
        EffectChainPresetPointer pPreset =
                m_effectChainPresets.value(presetName);
        return m_quickEffectChainPresetsSorted.indexOf(pPreset);
    }
    return -1;
}

int EffectChainPresetManager::quickEffectPresetIndex(
        EffectChainPresetPointer pPreset) const {
    return m_quickEffectChainPresetsSorted.indexOf(pPreset);
}

EffectChainPresetPointer EffectChainPresetManager::quickEffectPresetAtIndex(
        int index) const {
    if (index < 0) {
        index = m_quickEffectChainPresetsSorted.size() - 1;
    } else if (index >= m_quickEffectChainPresetsSorted.size()) {
        index = 0;
    }
    return m_quickEffectChainPresetsSorted.at(index);
}

bool EffectChainPresetManager::importPreset() {
    QStringList fileNames = QFileDialog::getOpenFileNames(nullptr,
            tr("Import effect chain preset"),
            QDir::homePath(),
            tr("Mixxx Effect Chain Presets") + QStringLiteral(" (*") +
                    kXmlFileExtension + QStringLiteral(")"));

    bool presetsImported = false;
    QString importFailedText = tr("Error importing effect chain preset");
    QString importFailedInformativeText = tr("Error importing effect chain preset \"%1\"");
    for (int i = 0; i < fileNames.size(); ++i) {
        QString filePath = fileNames.at(i);
        QDomDocument doc;
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox msgBox;
            msgBox.setText(importFailedText);
            msgBox.setInformativeText(importFailedInformativeText.arg(filePath));
            msgBox.setDetailedText(file.errorString());
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
            continue;
        } else if (!doc.setContent(&file)) {
            file.close();
            QMessageBox::critical(
                    nullptr, importFailedText, importFailedInformativeText.arg(filePath));
            continue;
        }
        file.close();

        auto pPreset = EffectChainPresetPointer::create(doc.documentElement());
        if (!pPreset->isEmpty() && !pPreset->name().isEmpty()) {
            // Don't allow '---' because that's the name of the internal empty preset
            if (pPreset->name() == kNoEffectString) {
                pPreset->setName(pPreset->name() +
                        QStringLiteral(" (") + tr("imported") + QStringLiteral(")"));
            }

            while (m_effectChainPresets.contains(pPreset->name())) {
                pPreset->setName(pPreset->name() +
                        QStringLiteral(" (") + tr("duplicate") + QStringLiteral(")"));
            }

            // An imported chain preset might contain an LV2 plugin that the user does not
            // have installed.
            for (const auto& pEffectPreset : pPreset->effectPresets()) {
                if (!pEffectPreset || pEffectPreset->isEmpty()) {
                    continue;
                }

                bool effectAvailable = false;
                for (const EffectManifestPointer& pManifest : m_pBackendManager->getManifests()) {
                    if (pManifest->id() == pEffectPreset->id() &&
                            pManifest->backendType() ==
                                    pEffectPreset->backendType()) {
                        effectAvailable = true;
                        break;
                    }
                }
                if (!effectAvailable) {
                    QMessageBox::warning(nullptr,
                            importFailedText,
                            tr("The effect chain imported from \"%1\" contains "
                               "an effect that is not available:")
                                            .arg(filePath) +
                                    QStringLiteral("\n\n") +
                                    pEffectPreset->id() +
                                    QStringLiteral("\n\n") +
                                    tr("If you load this chain preset, the "
                                       "unsupported effect will not be loaded "
                                       "with it."));
                }
            }

            if (!savePresetXml(pPreset)) {
                continue;
            }
            m_effectChainPresets.insert(pPreset->name(), pPreset);
            m_effectChainPresetsSorted.append(pPreset);
            m_quickEffectChainPresetsSorted.append(pPreset);
            emit effectChainPresetListUpdated();
            emit quickEffectChainPresetListUpdated();
            presetsImported = true;
        } else {
            QMessageBox::critical(
                    nullptr, importFailedText, importFailedInformativeText.arg(filePath));
        }
    }
    return presetsImported;
}

void EffectChainPresetManager::exportPreset(const QString& chainPresetName) {
    VERIFY_OR_DEBUG_ASSERT(m_effectChainPresets.contains(chainPresetName)) {
        return;
    }

    EffectChainPresetPointer pPreset =
            m_effectChainPresets.value(chainPresetName);
    VERIFY_OR_DEBUG_ASSERT(!pPreset->isEmpty()) {
        return;
    }

    QFileDialog saveFileDialog(
            nullptr,
            tr("Export effect chain preset"),
            QString(),
            tr("Mixxx Effect Chain Presets") + QStringLiteral(" (*") +
                    kXmlFileExtension + QStringLiteral(")"));
    saveFileDialog.setDefaultSuffix(kXmlFileExtension);
    saveFileDialog.setAcceptMode(QFileDialog::AcceptSave);
    saveFileDialog.selectFile(mixxx::filename::sanitize(pPreset->name()) + kXmlFileExtension);
    QString fileName;
    if (saveFileDialog.exec()) {
        fileName = saveFileDialog.selectedFiles().at(0);
    } else {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        QMessageBox msgBox;
        msgBox.setText(tr("Error exporting effect chain preset"));
        msgBox.setInformativeText(
                tr("Could not save effect chain preset \"%1\" to file \"%2\".")
                        .arg(chainPresetName, fileName));
        msgBox.setDetailedText(file.errorString());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }

    QDomDocument doc(EffectXml::kChain);
    doc.setContent(EffectXml::kFileHeader);
    doc.appendChild(pPreset->toXml(&doc));
    if (!file.write(doc.toByteArray())) {
        qWarning() << "Could not write effect chain preset XML to" << file.fileName();
    }
    file.close();
}

bool EffectChainPresetManager::renamePreset(const QString& oldName) {
    VERIFY_OR_DEBUG_ASSERT(m_effectChainPresets.contains(oldName)) {
        return false;
    }
    if (m_effectChainPresets.value(oldName)->isReadOnly()) {
        QMessageBox msgBox;
        msgBox.setText(tr("Effect chain preset can not be renamed"));
        msgBox.setInformativeText(
                tr("Effect chain preset \"%1\" is read-only and can not be renamed.")
                        .arg(oldName));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return false;
    }

    QString newName;
    QString errorText;
    // Don't allow '---' as new name either
    while (newName.isEmpty() || m_effectChainPresets.contains(newName) ||
            newName == kNoEffectString) {
        bool okay = false;
        newName = QInputDialog::getText(nullptr,
                tr("Rename effect chain preset"),
                errorText + "\n" + tr("New name for effect chain preset") +
                        QStringLiteral(" \"") + oldName + QStringLiteral("\""),
                QLineEdit::Normal,
                oldName,
                &okay)
                          .trimmed();
        if (!okay) {
            return false;
        }

        if (newName.isEmpty()) {
            errorText = tr("Effect chain preset name must not be empty.") + QStringLiteral("\n");
        } else if (newName == kNoEffectString) {
            errorText = tr("Invalid name \"%1\"").arg(newName) + QStringLiteral("\n");
        } else if (m_effectChainPresets.contains(newName)) {
            errorText =
                    tr("An effect chain preset named \"%1\" already exists.")
                            .arg(newName) +
                    QStringLiteral("\n");
        } else {
            errorText = QString();
        }
    }

    EffectChainPresetPointer pPreset = m_effectChainPresets.take(oldName);

    pPreset->setName(newName);
    m_effectChainPresets.insert(newName, pPreset);

    if (!savePresetXml(pPreset)) {
        // Saving failed, revert renaming
        m_effectChainPresets.take(newName);
        pPreset->setName(oldName);
        m_effectChainPresets.insert(oldName, pPreset);
        return false;
    }

    QString directoryPath = m_pConfig->getSettingsPath() + kEffectChainPresetDirectory;
    QFile oldFile(directoryPath + kFolderDelimiter +
            mixxx::filename::sanitize(oldName) + kXmlFileExtension);
    if (!oldFile.remove()) {
        QMessageBox msgBox;
        msgBox.setText(tr("Error removing old effect chain preset"));
        msgBox.setInformativeText(
                tr("Could not remove old effect chain preset \"%1\"")
                        .arg(oldName));
        msgBox.setDetailedText(oldFile.errorString());
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }

    emit effectChainPresetRenamed(oldName, newName);

    int index = m_effectChainPresetsSorted.indexOf(pPreset);
    if (index != -1) {
        m_effectChainPresetsSorted.replace(index, pPreset);
        emit effectChainPresetListUpdated();
    }
    index = m_quickEffectChainPresetsSorted.indexOf(pPreset);
    if (index != -1) {
        m_quickEffectChainPresetsSorted.replace(index, pPreset);
        emit quickEffectChainPresetListUpdated();
    }
    return true;
}

bool EffectChainPresetManager::deletePreset(const QString& chainPresetName) {
    VERIFY_OR_DEBUG_ASSERT(m_effectChainPresets.contains(chainPresetName)) {
        return false;
    }
    if (m_effectChainPresets.value(chainPresetName)->isReadOnly()) {
        QMessageBox msgBox;
        msgBox.setText(tr("Effect chain preset can not be deleted"));
        msgBox.setInformativeText(
                tr("Effect chain preset \"%1\" is read-only and can not be deleted.")
                        .arg(chainPresetName));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return false;
    }
    auto pressedButton = QMessageBox::question(nullptr,
            tr("Remove effect chain preset"),
            tr("Are you sure you want to delete the effect chain preset "
               "\"%1\"?")
                    .arg(chainPresetName));
    if (pressedButton != QMessageBox::Yes) {
        return false;
    }

    QFile file(m_pConfig->getSettingsPath() + kEffectChainPresetDirectory +
            kFolderDelimiter + mixxx::filename::sanitize(chainPresetName) + kXmlFileExtension);
    if (!file.remove()) {
        QMessageBox msgBox;
        msgBox.setText(tr("Error deleting effect chain preset"));
        msgBox.setInformativeText(
                tr("Could not delete effect chain preset \"%1\"")
                        .arg(chainPresetName));
        msgBox.setDetailedText(file.errorString());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return false;
    }

    EffectChainPresetPointer pPreset =
            m_effectChainPresets.take(chainPresetName);
    m_effectChainPresetsSorted.removeAll(pPreset);
    m_quickEffectChainPresetsSorted.removeAll(pPreset);
    emit effectChainPresetListUpdated();
    emit quickEffectChainPresetListUpdated();
    return true;
}

void EffectChainPresetManager::setPresetOrder(
        const QStringList& chainPresetList) {
    m_effectChainPresetsSorted.clear();

    for (const auto& chainPresetName : chainPresetList) {
        VERIFY_OR_DEBUG_ASSERT(m_effectChainPresets.contains(chainPresetName)) {
            continue;
        }
        m_effectChainPresetsSorted.append(
                m_effectChainPresets.value(chainPresetName));
    }

    // After having changed the presets order in DlgPrefEffects, we received the
    // new lists. The '---' was not displayed there, so it's not in the list.
    // Make sure the empty preset '---' is the first item in the list.
    const auto& pEmptyPreset = m_effectChainPresets.value(kNoEffectString);
    VERIFY_OR_DEBUG_ASSERT(pEmptyPreset) {
        return;
    }
    int index = m_effectChainPresetsSorted.indexOf(pEmptyPreset);
    if (index == -1) { // not in list, re-add it
        m_effectChainPresetsSorted.prepend(pEmptyPreset);
    } else if (index != 0) { // not first item, move to top
        m_effectChainPresetsSorted.move(index, 0);
    }

    emit effectChainPresetListUpdated();
}

void EffectChainPresetManager::setQuickEffectPresetOrder(
        const QStringList& chainPresetList) {
    m_quickEffectChainPresetsSorted.clear();

    for (const auto& chainPresetName : chainPresetList) {
        VERIFY_OR_DEBUG_ASSERT(m_effectChainPresets.contains(chainPresetName)) {
            continue;
        }
        m_quickEffectChainPresetsSorted.append(
                m_effectChainPresets.value(chainPresetName));
    }

    // After having changed the presets order in DlgPrefEffects, we received the
    // new lists. The '---' was not displayed there, so it's not in the list.
    // Make sure the empty preset '---' is the first item in the list.
    const auto& pEmptyPreset = m_effectChainPresets.value(kNoEffectString);
    VERIFY_OR_DEBUG_ASSERT(pEmptyPreset) {
        return;
    }
    int index = m_quickEffectChainPresetsSorted.indexOf(pEmptyPreset);
    if (index == -1) { // not in list, re-add it
        m_quickEffectChainPresetsSorted.prepend(pEmptyPreset);
    } else if (index != 0) { // not first item, move to top
        m_quickEffectChainPresetsSorted.move(index, 0);
    }

    emit quickEffectChainPresetListUpdated();
}

void EffectChainPresetManager::savePresetAndReload(EffectChainPointer pChainSlot) {
    auto pPreset = EffectChainPresetPointer::create(pChainSlot.data());
    if (savePreset(pPreset)) {
        pChainSlot->loadChainPreset(pPreset);
    }
}

bool EffectChainPresetManager::savePreset(EffectChainPresetPointer pPreset) {
    QString name;
    QString errorText;
    // Don't allow '---' because that's the name of the internal empty preset
    // Clear initial name to avoid confusion.
    QString presetName;
    if (pPreset->name() != kNoEffectString) {
        presetName = pPreset->name();
    }
    while (name.isEmpty() || m_effectChainPresets.contains(name) || name == kNoEffectString) {
        bool okay = false;
        name = QInputDialog::getText(nullptr,
                tr("Save preset for effect chain"),
                errorText + "\n" + tr("Name for new effect chain preset:"),
                QLineEdit::Normal,
                presetName,
                &okay)
                       .trimmed();
        if (!okay) {
            return false;
        }

        if (name.isEmpty()) {
            errorText = tr("Effect chain preset name must not be empty.") + QStringLiteral("\n");
        } else if (name == kNoEffectString) {
            errorText = tr("Invalid name \"%1\"").arg(name) + QStringLiteral("\n");
        } else if (m_effectChainPresets.contains(name)) {
            errorText =
                    tr("An effect chain preset named \"%1\" already exists.")
                            .arg(name) +
                    QStringLiteral("\n");
        } else {
            errorText = QString();
        }
    }

    pPreset->setName(name);
    m_effectChainPresets.insert(name, pPreset);
    m_effectChainPresetsSorted.append(pPreset);
    m_quickEffectChainPresetsSorted.append(pPreset);
    emit effectChainPresetListUpdated();
    emit quickEffectChainPresetListUpdated();

    savePresetXml(pPreset);
    return true;
}

void EffectChainPresetManager::updatePreset(EffectChainPointer pChainSlot) {
    const QString name = pChainSlot->presetName();
    if (name.isEmpty()) {
        return;
    }

    auto it = m_effectChainPresets.find(name);
    VERIFY_OR_DEBUG_ASSERT(it != m_effectChainPresets.end()) {
        return;
    }

    const int index = m_effectChainPresetsSorted.indexOf(*it);
    VERIFY_OR_DEBUG_ASSERT(index != -1) {
        return;
    }
    // Effect Chain Presets don't have a copy constructor, so we have to create a new one and
    // replace the existing entries in the two lists. Calling reset or swap on the iterator
    // also doesn't work.
    auto pPreset = EffectChainPresetPointer::create(pChainSlot.data());
    m_effectChainPresetsSorted.replace(index, pPreset);
    m_effectChainPresets.insert(name, pPreset);
    savePresetXml(pPreset);
}

void EffectChainPresetManager::importUserPresets() {
    QString savedPresetsPath(
            m_pConfig->getSettingsPath() + kEffectChainPresetDirectory);
    QDir savedPresetsDir(savedPresetsPath);
    if (!savedPresetsDir.exists()) {
        savedPresetsDir.mkpath(savedPresetsPath);
        return;
    }
    savedPresetsDir.setFilter(QDir::Files | QDir::Readable);
    const QStringList fileList = savedPresetsDir.entryList();
    for (const auto& filePath : fileList) {
        EffectChainPresetPointer pEffectChainPreset = loadPresetFromFile(
                savedPresetsPath + kFolderDelimiter + filePath);
        if (pEffectChainPreset && !pEffectChainPreset->isEmpty()) {
            // Don't allow '---' because that's the name of the internal empty preset
            if (pEffectChainPreset->name() == kNoEffectString) {
                pEffectChainPreset->setName(pEffectChainPreset->name() +
                        QLatin1String(" (") + tr("imported") + QLatin1String(")"));
            }
            m_effectChainPresets.insert(
                    pEffectChainPreset->name(), pEffectChainPreset);
        }
    }
}

void EffectChainPresetManager::importDefaultPresets() {
    QString savedPresetsPath(
            m_pConfig->getSettingsPath() + kEffectChainPresetDirectory);
    QString defaultPresetsPath(
            m_pConfig->getResourcePath() + kEffectChainPresetDirectory);
    QDir defaultChainPresetsDir(defaultPresetsPath);
    defaultChainPresetsDir.setFilter(QDir::Files | QDir::Readable);
    const auto& fileNames = defaultChainPresetsDir.entryList();
    for (const auto& fileName : fileNames) {
        QString copiedFileName = savedPresetsPath + kFolderDelimiter + fileName;
        QFileInfo copiedFileInfo(copiedFileName);
        if (!copiedFileInfo.exists()) {
            const QString originalFileName = defaultPresetsPath + kFolderDelimiter + fileName;
            if (!QFile::copy(originalFileName, copiedFileName)) {
                qWarning() << "Could not copy default effect chain preset to" << copiedFileName;
                continue;
            }
        }

        EffectChainPresetPointer pEffectChainPreset = loadPresetFromFile(copiedFileName);
        if (pEffectChainPreset && !pEffectChainPreset->isEmpty()) {
            m_effectChainPresets.insert(pEffectChainPreset->name(), pEffectChainPreset);
            m_effectChainPresetsSorted.append(pEffectChainPreset);
        } else {
            qWarning() << "Could not load default effect chain preset" << copiedFileName;
        }
    }
}

void EffectChainPresetManager::generateDefaultQuickEffectPresets() {
    // importDefaultPresets should be called before this function
    DEBUG_ASSERT(!m_effectChainPresetsSorted.isEmpty());

    for (const auto& presetName : std::as_const(m_effectChainPresetsSorted)) {
        m_quickEffectChainPresetsSorted.append(presetName);
    }
    // On first run of Mixxx, generate QuickEffect chain presets for every
    // effect with a default metaknob linking. These are generated rather
    // than included with Mixxx in res/effects/chains so the translated
    // names are used.
    QVector<EffectManifestPointer> manifestList;
    for (const auto& pManifest : m_pBackendManager->getManifests()) {
        if (pManifest->hasMetaKnobLinking()) {
            manifestList.append(pManifest);
        }
    }
    std::sort(manifestList.begin(), manifestList.end(), EffectManifest::sortLexigraphically);
    for (const auto& pManifest : manifestList) {
        auto pChainPreset = EffectChainPresetPointer::create(pManifest);
        pChainPreset->setName(pManifest->displayName());
        m_effectChainPresets.insert(pChainPreset->name(), pChainPreset);
        m_quickEffectChainPresetsSorted.append(pChainPreset);
        savePresetXml(pChainPreset);
    }
}

void EffectChainPresetManager::prependRemainingPresetsToLists() {
    // This function should be called after these lists are populated.
    DEBUG_ASSERT(!m_effectChainPresetsSorted.isEmpty());
    DEBUG_ASSERT(!m_quickEffectChainPresetsSorted.isEmpty());

    // If there are any preset files in the user settings folder that are not
    // in either preset list, the user has placed them there since the last
    // time Mixxx was run. Prepend these presets to the lists for both the
    // regular effect units and QuickEffects.
    for (const EffectChainPresetPointer& pPreset : std::as_const(m_effectChainPresets)) {
        if (!m_effectChainPresetsSorted.contains(pPreset) &&
                !m_quickEffectChainPresetsSorted.contains(pPreset)) {
            m_effectChainPresetsSorted.prepend(pPreset);
            m_quickEffectChainPresetsSorted.prepend(pPreset);
        }
    }
}

void EffectChainPresetManager::resetToDefaults() {
    m_effectChainPresetsSorted.clear();
    m_quickEffectChainPresetsSorted.clear();
    m_effectChainPresets.clear();

    importUserPresets();
    importDefaultPresets();
    generateDefaultQuickEffectPresets();
    prependRemainingPresetsToLists();

    // Re-add the empty chain preset
    EffectChainPresetPointer pEmptyChainPreset = createEmptyReadOnlyChainPreset();
    m_effectChainPresets.insert(pEmptyChainPreset->name(), pEmptyChainPreset);
    m_effectChainPresetsSorted.prepend(pEmptyChainPreset);
    m_quickEffectChainPresetsSorted.prepend(pEmptyChainPreset);

    emit effectChainPresetListUpdated();
    emit quickEffectChainPresetListUpdated();
}

bool EffectChainPresetManager::savePresetXml(EffectChainPresetPointer pPreset) {
    // Don't store the empty '---' preset in effects/chains.
    // Shouldn't be possible via GUI anyway because the 'Update Preset' button
    // is not shown in WEffectChainPresetButton if this preset is loaded.
    DEBUG_ASSERT(pPreset->name() != kNoEffectString);

    QString path(m_pConfig->getSettingsPath() + kEffectChainPresetDirectory);
    QDir effectsChainsDir(path);
    if (!effectsChainsDir.exists()) {
        effectsChainsDir.mkpath(path);
    }
    // The file name does not matter as long as it is unique. The actual name string
    // is safely stored in the UTF8 document, regardless of what the filesystem
    // supports for file names.
    QFile file(path + kFolderDelimiter +
            mixxx::filename::sanitize(pPreset->name()) + kXmlFileExtension);
    if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        QMessageBox msgBox;
        msgBox.setText(tr("Error saving effect chain preset"));
        msgBox.setInformativeText(
                tr("Could not save effect chain preset \"%1\"")
                        .arg(pPreset->name()));
        msgBox.setDetailedText(file.errorString());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        qWarning() << "Could not save effect chain preset" << file.fileName();
        return false;
    }

    QDomDocument doc(EffectXml::kChain);
    doc.setContent(EffectXml::kFileHeader);
    doc.appendChild(pPreset->toXml(&doc));
    const bool success = file.write(doc.toByteArray());
    if (!success) {
        qWarning() << "Could not write effect chain preset XML to" << file.fileName();
    }
    file.close();
    return success;
}

// static
EffectChainPresetPointer EffectChainPresetManager::createEmptyNamelessChainPreset() {
    auto pPreset = EffectChainPresetPointer::create(EffectChainPreset());
    pPreset->setName(kNoEffectString);
    return pPreset;
}

EffectManifestPointer EffectChainPresetManager::getDefaultEqEffect() {
    EffectManifestPointer pDefaultEqEffect = m_pBackendManager->getManifest(
            BiquadFullKillEQEffect::getId(), EffectBackendType::BuiltIn);
    DEBUG_ASSERT(!pDefaultEqEffect.isNull());
    return pDefaultEqEffect;
}

EffectChainPresetPointer EffectChainPresetManager::getDefaultQuickEffectPreset() {
    EffectManifestPointer pDefaultQuickEffectManifest = m_pBackendManager->getManifest(
            FilterEffect::getId(), EffectBackendType::BuiltIn);
    auto defaultQuickEffectChainPreset =
            EffectChainPresetPointer(pDefaultQuickEffectManifest
                            ? new EffectChainPreset(pDefaultQuickEffectManifest)
                            : new EffectChainPreset());
    return defaultQuickEffectChainPreset;
}

EffectsXmlData EffectChainPresetManager::readEffectsXml(
        const QDomDocument& doc, const QStringList& deckStrings) {
    auto pDefaultEqEffect = getDefaultEqEffect();
    auto defaultQuickEffectChainPreset = getDefaultQuickEffectPreset();

    QList<EffectChainPresetPointer> standardEffectChainPresets;
    QHash<QString, EffectChainPresetPointer> quickEffectPresets;
    QHash<QString, EffectChainPresetPointer> quickStemEffectPresets;
    QHash<QString, EffectManifestPointer> eqEffectManifests;
    // configure default EQs and QuickEffects per deck
    for (const auto& deckString : deckStrings) {
        quickEffectPresets.insert(deckString, defaultQuickEffectChainPreset);
        eqEffectManifests.insert(deckString, pDefaultEqEffect);
    }

    // Read state of standard chains
    QDomElement root = doc.documentElement();
    QDomElement rackElement = XmlParse::selectElement(root, EffectXml::kRack);
    QDomElement chainsElement =
            XmlParse::selectElement(rackElement, EffectXml::kChainsRoot);
    const QDomNodeList chainsList = chainsElement.elementsByTagName(EffectXml::kChain);

    for (int i = 0; i < chainsList.count(); ++i) {
        QDomNode chainNode = chainsList.at(i);

        if (chainNode.isElement()) {
            QDomElement chainElement = chainNode.toElement();
            EffectChainPresetPointer pPreset =
                    EffectChainPresetPointer::create(chainElement);
            // Shouldn't happen, see EffectSlot::loadEffectInner
            VERIFY_OR_DEBUG_ASSERT(pPreset->name() != kNoEffectString) {
                pPreset->setName("");
            }
            standardEffectChainPresets.append(pPreset);
        }
    }

    QDomElement mainEqElement = XmlParse::selectElement(root, EffectXml::kMainEq);
    QDomNodeList mainEqs = mainEqElement.elementsByTagName(EffectXml::kChain);
    QDomNode mainEqChainNode = mainEqs.at(0);
    EffectChainPresetPointer mainEqPreset = nullptr;
    if (mainEqChainNode.isElement()) {
        QDomElement mainEqChainElement = mainEqChainNode.toElement();
        mainEqPreset = EffectChainPresetPointer::create(mainEqChainElement);
    }

    importUserPresets();

    // Reload order of custom chain presets
    QDomElement chainPresetsElement =
            XmlParse::selectElement(root, EffectXml::kChainPresetList);
    const QDomNodeList presetNameList =
            chainPresetsElement.elementsByTagName(EffectXml::kChainPresetName);
    for (int i = 0; i < presetNameList.count(); ++i) {
        QDomNode presetNameNode = presetNameList.at(i);
        if (presetNameNode.isElement()) {
            QString presetName = presetNameNode.toElement().text();
            auto pPreset = m_effectChainPresets.value(presetName);
            if (!pPreset) {
                qWarning() << presetName
                           << "specified in list of effect chain presets in "
                              "effects.xml but no such preset was found.";
                continue;
            }
            m_effectChainPresetsSorted.append(pPreset);
        }
    }

    if (m_effectChainPresetsSorted.isEmpty()) {
        // On first run of Mixxx, copy chain presets from the resource folder to the
        // user settings folder. This allows us to ship default chain presets with
        // Mixxx while letting the user delete our defaults if they do not want them.
        // The user can always copy the files again manually or use the Import
        // button in DlgPrefEffects.
        importDefaultPresets();
    }

    // Reload order of QuickEffect chain presets
    QDomElement quickEffectChainPresetsElement =
            XmlParse::selectElement(root, EffectXml::kQuickEffectList);
    const QDomNodeList quickEffectPresetNameList =
            quickEffectChainPresetsElement.elementsByTagName(EffectXml::kChainPresetName);
    for (int i = 0; i < quickEffectPresetNameList.count(); ++i) {
        QDomNode presetNameNode = quickEffectPresetNameList.at(i);
        if (presetNameNode.isElement()) {
            QString presetName = presetNameNode.toElement().text();
            auto pPreset = m_effectChainPresets.value(presetName);
            if (!pPreset) {
                qWarning() << presetName
                           << "specified in list of QuickEffect chain presets "
                              "in effects.xml but no such preset was found.";
                continue;
            }
            m_quickEffectChainPresetsSorted.append(pPreset);
        }
    }

    if (m_quickEffectChainPresetsSorted.empty()) {
        generateDefaultQuickEffectPresets();
    }

    prependRemainingPresetsToLists();

    // Create the empty '---' chain preset on each start.
    // Its sole purpose is to eject the current QuickEffect chain presets via GUI.
    // It will not be saved to effects/chains nor written to effects.xml
    // except as identifier for QuickEffect chains.
    // It will not be visible in the effects preferences.
    // Note: we also need to take care of this preset in resetToDefaults() and
    // setQuickEffectPresetOrder(), both called from DlgPrefEffects
    EffectChainPresetPointer pEmptyChainPreset = createEmptyReadOnlyChainPreset();
    m_effectChainPresets.insert(pEmptyChainPreset->name(), pEmptyChainPreset);
    m_effectChainPresetsSorted.prepend(pEmptyChainPreset);
    m_quickEffectChainPresetsSorted.prepend(pEmptyChainPreset);

    emit effectChainPresetListUpdated();
    emit quickEffectChainPresetListUpdated();

    // Read ids of effects that were loaded into Equalizer slots on last shutdown
    QDomElement eqEffectsElement =
            XmlParse::selectElement(root, EffectXml::kEqualizerEffects);
    const QDomNodeList eqEffectNodeList =
            eqEffectsElement.elementsByTagName(
                    EffectXml::kEffectId);
    for (int i = 0; i < eqEffectNodeList.count(); ++i) {
        QDomElement effectIdElement = eqEffectNodeList.at(i).toElement();
        if (!effectIdElement.isNull()) {
            QString deckGroup = effectIdElement.attribute(QStringLiteral("group"));
            QString uid = effectIdElement.text();
            auto pManifest = m_pBackendManager->getManifestFromUniqueId(uid);
            // Replace default EQ effect with pManifest for this deck group.
            // Also load empty manifests to restore empty EQ effects.
            if (pManifest != nullptr || uid == kNoEffectString) {
                eqEffectManifests.insert(deckGroup, pManifest);
            }
        }
    }

    // Read names of presets that were loaded into QuickEffects on last shutdown
    QDomElement quickEffectPresetsElement =
            XmlParse::selectElement(root, EffectXml::kQuickEffectChainPresets);
    const QDomNodeList quickEffectNodeList =
            quickEffectPresetsElement.elementsByTagName(
                    EffectXml::kChainPresetName);
    for (int i = 0; i < quickEffectNodeList.count(); ++i) {
        QDomElement presetNameElement = quickEffectNodeList.at(i).toElement();
        if (!presetNameElement.isNull()) {
            QString deckGroup = presetNameElement.attribute(QStringLiteral("group"));
            auto pPreset = m_effectChainPresets.value(presetNameElement.text());
            if (pPreset != nullptr) {
                // Replace defaultQuickEffectChainPreset with pPreset
                // for this deck group
                quickEffectPresets.insert(deckGroup, pPreset);
            }
        }
    }

    // Read names of presets that were loaded into stem QuickEffects on last shutdown
    QDomElement quickStemEffectPresetsElement =
            XmlParse::selectElement(root, EffectXml::kStemQuickEffectChainPresets);
    const QDomNodeList quickStemEffectNodeList =
            quickStemEffectPresetsElement.elementsByTagName(
                    EffectXml::kChainPresetName);
    quickStemEffectPresets.reserve(quickStemEffectNodeList.count());
    for (int i = 0; i < quickStemEffectNodeList.count(); ++i) {
        QDomElement presetNameElement = quickStemEffectNodeList.at(i).toElement();
        if (!presetNameElement.isNull()) {
            QString deckStemGroup = presetNameElement.attribute(QStringLiteral("group"));
            auto pPreset = m_effectChainPresets.value(presetNameElement.text());
            if (pPreset != nullptr) {
                // Replace defaultQuickEffectChainPreset with pPreset
                // for this deck group
                quickStemEffectPresets.insert(deckStemGroup, pPreset);
            }
        }
    }

    return EffectsXmlData{eqEffectManifests,
            quickEffectPresets,
            quickStemEffectPresets,
            standardEffectChainPresets,
            mainEqPreset};
}

EffectXmlDataSingleDeck EffectChainPresetManager::readEffectsXmlSingleDeck(
        const QDomDocument& doc, const QString& deckString) {
    QDomElement root = doc.documentElement();

    // EQ effect
    auto pEqEffect = getDefaultEqEffect();

    // Read id of last loaded EQ effect
    QDomElement eqEffectsElement =
            XmlParse::selectElement(root, EffectXml::kEqualizerEffects);
    const QDomNodeList eqEffectNodeList =
            eqEffectsElement.elementsByTagName(
                    EffectXml::kEffectId);
    for (int i = 0; i < eqEffectNodeList.count(); ++i) {
        QDomElement effectIdElement = eqEffectNodeList.at(i).toElement();
        if (effectIdElement.isNull()) {
            continue;
        }
        if (effectIdElement.attribute(QStringLiteral("group")) == deckString) {
            QString uid = effectIdElement.text();
            auto pManifest = m_pBackendManager->getManifestFromUniqueId(uid);
            // Replace the default EQ effect.
            // Load empty effect if the slot was cleared explicitly ('---' preset)
            if (pManifest != nullptr || uid == kNoEffectString) {
                pEqEffect = pManifest;
            }
        }
    }

    // Quick Effect
    auto pQuickEffectChainPreset = getDefaultQuickEffectPreset();

    // Read name of last loaded QuickEffect preset
    QDomElement quickEffectPresetsElement =
            XmlParse::selectElement(root, EffectXml::kQuickEffectChainPresets);
    const QDomNodeList quickEffectNodeList =
            quickEffectPresetsElement.elementsByTagName(
                    EffectXml::kChainPresetName);
    for (int i = 0; i < quickEffectNodeList.count(); ++i) {
        QDomElement presetNameElement = quickEffectNodeList.at(i).toElement();
        if (presetNameElement.isNull()) {
            continue;
        }
        if (presetNameElement.attribute(QStringLiteral("group")) == deckString) {
            auto pPreset = m_effectChainPresets.value(presetNameElement.text());
            if (pPreset != nullptr || presetNameElement.text() == kNoEffectString) {
                // Replace the default preset.
                // Load empty preset if the chain was cleared explicitly ('---' preset)
                pQuickEffectChainPreset = pPreset;
            }
        }
    }

    return EffectXmlDataSingleDeck{pEqEffect, pQuickEffectChainPreset};
}

EffectChainPresetPointer EffectChainPresetManager::readEffectsXmlSingleDeckStem(
        const QDomDocument& doc, const QString& deckStemString) {
    QDomElement root = doc.documentElement();

    // Quick Effect
    auto pQuickEffectChainPreset = getDefaultQuickEffectPreset();

    // Read name of last loaded QuickEffect preset
    QDomElement quickEffectPresetsElement =
            XmlParse::selectElement(root, EffectXml::kStemQuickEffectChainPresets);
    const QDomNodeList quickEffectNodeList =
            quickEffectPresetsElement.elementsByTagName(
                    EffectXml::kChainPresetName);
    for (int i = 0; i < quickEffectNodeList.count(); ++i) {
        QDomElement presetNameElement = quickEffectNodeList.at(i).toElement();
        if (presetNameElement.isNull()) {
            continue;
        }
        if (presetNameElement.attribute(QStringLiteral("group")) == deckStemString) {
            auto pPreset = m_effectChainPresets.value(presetNameElement.text());
            if (pPreset != nullptr || presetNameElement.text() == kNoEffectString) {
                // Replace the default preset.
                // Load empty preset if the chain was cleared explicitly ('---' preset)
                pQuickEffectChainPreset = pPreset;
            }
        }
    }

    return pQuickEffectChainPreset;
}

void EffectChainPresetManager::saveEffectsXml(QDomDocument* pDoc, const EffectsXmlData& data) {
    // Save presets for current state of standard chains
    QDomElement rootElement = pDoc->documentElement();
    QDomElement rackElement = pDoc->createElement(EffectXml::kRack);
    rootElement.appendChild(rackElement);
    QDomElement chainsElement = pDoc->createElement(EffectXml::kChainsRoot);
    rackElement.appendChild(chainsElement);
    for (const auto& pPreset : std::as_const(data.standardEffectChainPresets)) {
        // Don't store the empty '---' preset.
        if (pPreset->name() == kNoEffectString) {
            // It must not have any effects loaded. If it has, clear the name.
            // See readEffectsXml() for explanation.
            VERIFY_OR_DEBUG_ASSERT(pPreset->isEmpty()) {
                pPreset->setName("");
            }
            else {
                continue;
            }
        }
        chainsElement.appendChild(pPreset->toXml(pDoc));
    }

    // Save main EQ effect
    QDomElement mainEqElement = pDoc->createElement(EffectXml::kMainEq);
    rootElement.appendChild(mainEqElement);
    const auto& mainEqPreset = data.outputChainPreset;
    mainEqElement.appendChild(mainEqPreset->toXml(pDoc));

    // Save order of custom chain presets
    QDomElement chainPresetListElement =
            pDoc->createElement(EffectXml::kChainPresetList);
    for (const auto& pPreset : std::as_const(m_effectChainPresetsSorted)) {
        // Don't store the empty '---' preset in the main preset list,
        // it won't be exported anyway.
        if (pPreset->name() == kNoEffectString) {
            continue;
        }
        XmlParse::addElement(*pDoc,
                chainPresetListElement,
                EffectXml::kChainPresetName,
                pPreset->name());
    }
    rootElement.appendChild(chainPresetListElement);

    // Save order of QuickEffect chain presets
    QDomElement quickEffectChainPresetListElement =
            pDoc->createElement(EffectXml::kQuickEffectList);
    for (const auto& pPreset : std::as_const(m_quickEffectChainPresetsSorted)) {
        // Same here, don't store the empty '---' preset
        if (pPreset->name() == kNoEffectString) {
            continue;
        }
        XmlParse::addElement(*pDoc,
                quickEffectChainPresetListElement,
                EffectXml::kChainPresetName,
                pPreset->name());
    }
    rootElement.appendChild(quickEffectChainPresetListElement);

    // Save ids of effects loaded to slot 1 of EQ chains
    QDomElement eqEffectsElement =
            pDoc->createElement(EffectXml::kEqualizerEffects);
    QHashIterator<QString, EffectManifestPointer> eqIt(data.eqEffectManifests);
    while (eqIt.hasNext()) {
        eqIt.next();
        // Save element with '---' if no EQ is loaded
        QString uid = eqIt.value().isNull() ? kNoEffectString : eqIt.value()->uniqueId();
        QDomElement eqEffectElement = XmlParse::addElement(
                *pDoc,
                eqEffectsElement,
                EffectXml::kEffectId,
                uid);
        eqEffectElement.setAttribute(QStringLiteral("group"), eqIt.key());
    }
    rootElement.appendChild(eqEffectsElement);

    // Save which presets are loaded to QuickEffects
    QDomElement quickEffectPresetsElement =
            pDoc->createElement(EffectXml::kQuickEffectChainPresets);
    QHashIterator<QString, EffectChainPresetPointer> qeIt(data.quickEffectChainPresets);
    while (qeIt.hasNext()) {
        qeIt.next();
        QDomElement quickEffectElement = XmlParse::addElement(
                *pDoc,
                quickEffectPresetsElement,
                EffectXml::kChainPresetName,
                qeIt.value()->name());
        quickEffectElement.setAttribute(QStringLiteral("group"), qeIt.key());
    }
    rootElement.appendChild(quickEffectPresetsElement);

    // Save which presets are loaded to stem QuickEffects
    QDomElement quickStemEffectPresetsElement =
            pDoc->createElement(EffectXml::kStemQuickEffectChainPresets);
    QHashIterator<QString, EffectChainPresetPointer> qseIt(data.quickStemEffectChainPresets);
    while (qseIt.hasNext()) {
        qseIt.next();
        QDomElement quickEffectElement = XmlParse::addElement(
                *pDoc,
                quickStemEffectPresetsElement,
                EffectXml::kChainPresetName,
                qseIt.value()->name());
        quickEffectElement.setAttribute(QStringLiteral("group"), qseIt.key());
    }
    rootElement.appendChild(quickStemEffectPresetsElement);
}
