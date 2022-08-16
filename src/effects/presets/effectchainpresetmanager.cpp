#include "effects/presets/effectchainpresetmanager.h"

#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

#include "effects/backends/builtin/filtereffect.h"
#include "effects/effectchain.h"
#include "effects/effectsmanager.h"
#include "effects/presets/effectxmlelements.h"
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
    EffectChainPresetPointer pEffectChainPreset(
            new EffectChainPreset(doc.documentElement()));
    file.close();
    return pEffectChainPreset;
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

void EffectChainPresetManager::importPreset() {
    QStringList fileNames = QFileDialog::getOpenFileNames(nullptr,
            tr("Import effect chain preset"),
            QDir::homePath(),
            tr("Mixxx Effect Chain Presets") + QStringLiteral(" (*") +
                    kXmlFileExtension + QStringLiteral(")"));

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

        EffectChainPresetPointer pPreset(
                new EffectChainPreset(doc.documentElement()));
        if (!pPreset->isEmpty() && !pPreset->name().isEmpty()) {
            // Don't allow '---' because it is used internally for clearing effect
            // chains and empty Quick Effect chains
            if (pPreset->name() == kNoEffectString) {
                pPreset->setName(pPreset->name() +
                        QLatin1String(" (") + tr("imported") + QLatin1String(")"));
            }

            while (m_effectChainPresets.contains(pPreset->name())) {
                pPreset->setName(pPreset->name() +
                        QLatin1String(" (") + tr("duplicate") + QLatin1String(")"));
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
        } else {
            QMessageBox::critical(
                    nullptr, importFailedText, importFailedInformativeText.arg(filePath));
        }
    }
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

void EffectChainPresetManager::renamePreset(const QString& oldName) {
    VERIFY_OR_DEBUG_ASSERT(m_effectChainPresets.contains(oldName)) {
        return;
    }

    QString newName;
    QString errorText;
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
            return;
        }

        if (newName.isEmpty()) {
            errorText = tr("Effect chain preset name must not be empty.") + QStringLiteral("\n");
        } else if (m_effectChainPresets.contains(newName)) {
            errorText =
                    tr("An effect chain preset named \"%1\" already exists.")
                            .arg(newName) +
                    QStringLiteral("\n");
        } else if (newName == kNoEffectString) {
            errorText = tr("Invalid name \"%1\"").arg(newName) + QStringLiteral("\n");
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
        return;
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
}

bool EffectChainPresetManager::deletePreset(const QString& chainPresetName) {
    VERIFY_OR_DEBUG_ASSERT(m_effectChainPresets.contains(chainPresetName)) {
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

    emit quickEffectChainPresetListUpdated();
}

void EffectChainPresetManager::savePreset(EffectChainPointer pChainSlot) {
    auto pPreset = EffectChainPresetPointer::create(pChainSlot.data());
    savePreset(pPreset);
}

void EffectChainPresetManager::savePreset(EffectChainPresetPointer pPreset) {
    QString name;
    QString errorText;
    while (name.isEmpty() || m_effectChainPresets.contains(name) || name == kNoEffectString) {
        bool okay = false;
        name = QInputDialog::getText(nullptr,
                tr("Save preset for effect chain"),
                errorText + "\n" + tr("Name for new effect chain preset:"),
                QLineEdit::Normal,
                pPreset->name(),
                &okay)
                       .trimmed();
        if (!okay) {
            return;
        }

        if (name.isEmpty()) {
            errorText = tr("Effect chain preset name must not be empty.") + QStringLiteral("\n");
        } else if (m_effectChainPresets.contains(name)) {
            errorText =
                    tr("An effect chain preset named \"%1\" already exists.")
                            .arg(name) +
                    QStringLiteral("\n");
        } else if (name == kNoEffectString) {
            errorText = tr("Invalid name \"%1\"").arg(name) + QStringLiteral("\n");
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
            // Don't allow '---' because it is used internally for clearing effect
            // chains and empty Quick Effect chains
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
        auto pChainPreset = EffectChainPresetPointer(new EffectChainPreset(pManifest));
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

    emit effectChainPresetListUpdated();
    emit quickEffectChainPresetListUpdated();
}

bool EffectChainPresetManager::savePresetXml(EffectChainPresetPointer pPreset) {
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

EffectsXmlData EffectChainPresetManager::readEffectsXml(
        const QDomDocument& doc, const QStringList& deckStrings) {
    EffectManifestPointer pDefaultQuickEffectManifest = m_pBackendManager->getManifest(
            FilterEffect::getId(), EffectBackendType::BuiltIn);
    auto defaultQuickEffectChainPreset =
            EffectChainPresetPointer(pDefaultQuickEffectManifest
                            ? new EffectChainPreset(pDefaultQuickEffectManifest)
                            : new EffectChainPreset());

    QList<EffectChainPresetPointer> standardEffectChainPresets;
    QHash<QString, EffectChainPresetPointer> quickEffectPresets;
    for (const auto& deckString : deckStrings) {
        quickEffectPresets.insert(deckString, defaultQuickEffectChainPreset);
    }

    // Reload state of standard chains
    QDomElement root = doc.documentElement();
    QDomElement rackElement = XmlParse::selectElement(root, EffectXml::kRack);
    QDomElement chainsElement =
            XmlParse::selectElement(rackElement, EffectXml::kChainsRoot);
    QDomNodeList chainsList = chainsElement.elementsByTagName(EffectXml::kChain);

    for (int i = 0; i < chainsList.count(); ++i) {
        QDomNode chainNode = chainsList.at(i);

        if (chainNode.isElement()) {
            QDomElement chainElement = chainNode.toElement();
            EffectChainPresetPointer pPreset(
                    new EffectChainPreset(chainElement));
            standardEffectChainPresets.append(pPreset);
        }
    }

    importUserPresets();

    // Reload order of custom chain presets
    QStringList chainPresetsSorted;
    QDomElement chainPresetsElement =
            XmlParse::selectElement(root, EffectXml::kChainPresetList);
    QDomNodeList presetNameList =
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
    QStringList quickEffectChainPresetsSorted;
    QDomElement quickEffectChainPresetsElement =
            XmlParse::selectElement(root, EffectXml::kQuickEffectList);
    QDomNodeList quickEffectPresetNameList =
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

    emit effectChainPresetListUpdated();
    emit quickEffectChainPresetListUpdated();

    // Reload presets that were loaded into QuickEffects on last shutdown
    QDomElement quickEffectPresetsElement =
            XmlParse::selectElement(root, EffectXml::kQuickEffectChainPresets);
    QDomNodeList quickEffectNodeList =
            quickEffectPresetsElement.elementsByTagName(
                    EffectXml::kChainPresetName);
    for (int i = 0; i < quickEffectNodeList.count(); ++i) {
        QDomElement presetNameElement = quickEffectNodeList.at(i).toElement();
        if (!presetNameElement.isNull()) {
            QString deckGroup = presetNameElement.attribute(QStringLiteral("group"));
            auto pPreset = m_effectChainPresets.value(presetNameElement.text());
            if (pPreset != nullptr) {
                quickEffectPresets.insert(deckGroup, pPreset);
            }
        }
    }

    return EffectsXmlData{quickEffectPresets, standardEffectChainPresets};
}

void EffectChainPresetManager::saveEffectsXml(QDomDocument* pDoc, const EffectsXmlData& data) {
    // Save presets for current state of standard chains
    QDomElement rootElement = pDoc->documentElement();
    QDomElement rackElement = pDoc->createElement(EffectXml::kRack);
    rootElement.appendChild(rackElement);
    QDomElement chainsElement = pDoc->createElement(EffectXml::kChainsRoot);
    rackElement.appendChild(chainsElement);
    for (const auto& pPreset : std::as_const(data.standardEffectChainPresets)) {
        chainsElement.appendChild(pPreset->toXml(pDoc));
    }

    // Save order of custom chain presets
    QDomElement chainPresetListElement =
            pDoc->createElement(EffectXml::kChainPresetList);
    for (const auto& pPreset : std::as_const(m_effectChainPresetsSorted)) {
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
        XmlParse::addElement(*pDoc,
                quickEffectChainPresetListElement,
                EffectXml::kChainPresetName,
                pPreset->name());
    }
    rootElement.appendChild(quickEffectChainPresetListElement);

    // Save which presets are loaded to QuickEffects
    QDomElement quickEffectPresetsElement =
            pDoc->createElement(EffectXml::kQuickEffectChainPresets);
    for (auto it = data.quickEffectChainPresets.begin();
            it != data.quickEffectChainPresets.end();
            it++) {
        QDomElement quickEffectElement = XmlParse::addElement(
                *pDoc,
                quickEffectPresetsElement,
                EffectXml::kChainPresetName,
                it.value()->name());
        quickEffectElement.setAttribute(QStringLiteral("group"), it.key());
    }
    rootElement.appendChild(quickEffectPresetsElement);
}
