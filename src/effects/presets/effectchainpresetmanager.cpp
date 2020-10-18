#include "effects/presets/effectchainpresetmanager.h"

#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

#include "effects/backends/builtin/filtereffect.h"
#include "effects/effectsmanager.h"
#include "effects/presets/effectxmlelements.h"
#include "util/filename.h"
#include "util/xml.h"

namespace {
const QString kEffectChainPresetDirectory = QStringLiteral("/effects/chains");
const QString kXmlFileExtension = QStringLiteral(".xml");
const QString kFolderDelimiter = QStringLiteral("/");
} // anonymous namespace

EffectChainPresetManager::EffectChainPresetManager(UserSettingsPointer pConfig,
        EffectsBackendManagerPointer pBackendManager)
        : m_pConfig(pConfig),
          m_pBackendManager(pBackendManager) {
    loadEffectChainPresets();
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
            QString(),
            tr("Mixxx Effect Chain Presets") + QStringLiteral(" (*") +
                    kXmlFileExtension + QStringLiteral(")"));

    QString importFailed = tr("Error importing effect chain preset");
    for (int i = 0; i < fileNames.size(); ++i) {
        QString filePath = fileNames.at(i);
        QDomDocument doc;
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(
                    nullptr, importFailed, importFailed + QStringLiteral(" ") + filePath);
            continue;
        } else if (!doc.setContent(&file)) {
            file.close();
            QMessageBox::critical(
                    nullptr, importFailed, importFailed + QStringLiteral(" ") + filePath);
            continue;
        }
        file.close();

        EffectChainPresetPointer pPreset(
                new EffectChainPreset(doc.documentElement()));
        if (!pPreset->isEmpty()) {
            if (m_effectChainPresets.contains(pPreset->name())) {
                bool okay = false;
                QString newName = QInputDialog::getText(nullptr,
                        tr("Rename effect chain preset"),
                        tr("An effect chain preset with the name") + QStringLiteral(" \"") +
                                pPreset->name() + QStringLiteral("\" ") +
                                tr("already exists. Choose a new name for the "
                                   "imported effect chain preset:"),
                        QLineEdit::Normal,
                        QString(),
                        &okay);
                if (!okay) {
                    continue;
                }
                pPreset->setName(newName);
            }

            // An imported chain preset might contain an LV2 plugin that the user does not
            // have installed.
            for (const auto& pEffectPreset : pPreset->effectPresets()) {
                if (!pEffectPreset || pEffectPreset->isEmpty()) {
                    continue;
                }

                bool effectSupported = false;
                for (EffectManifestPointer pManifest : m_pBackendManager->getManifests()) {
                    if (pManifest->id() == pEffectPreset->id() &&
                            pManifest->backendType() ==
                                    pEffectPreset->backendType()) {
                        effectSupported = true;
                        break;
                    }
                }
                if (!effectSupported) {
                    QMessageBox::critical(nullptr,
                            importFailed,
                            tr("The effect chain imported from") +
                                    QStringLiteral(" ") + filePath +
                                    QStringLiteral(" ") +
                                    tr("contains an effect that Mixxx does not "
                                       "support") +
                                    QStringLiteral(":\n\n") +
                                    pEffectPreset->id() +
                                    QStringLiteral("\n\n") +
                                    tr("If you load this chain preset, the "
                                       "unsupported effect will not be loaded "
                                       "with it."));
                }
            }

            m_effectChainPresets.insert(pPreset->name(), pPreset);
            m_effectChainPresetsSorted.append(pPreset);
            m_quickEffectChainPresetsSorted.append(pPreset);
            emit effectChainPresetListUpdated();
            emit quickEffectChainPresetListUpdated();
        } else {
            QMessageBox::critical(
                    nullptr, importFailed, importFailed + " " + filePath);
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

    // TODO: replace with QFileDialog::saveFileDialog when
    // https://bugreports.qt.io/browse/QTBUG-27186 is fixed.
    QFileDialog saveFileDialog(
            nullptr,
            tr("Save effect chain preset"),
            QString(),
            tr("Mixxx Effect Chain Presets") + QStringLiteral(" (*") +
                    kXmlFileExtension + QStringLiteral(")"));
    saveFileDialog.setDefaultSuffix(kXmlFileExtension);
    saveFileDialog.setAcceptMode(QFileDialog::AcceptSave);
    QString fileName;
    if (saveFileDialog.exec()) {
        fileName = saveFileDialog.selectedFiles().at(0);
    } else {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        file.close();
        QMessageBox::critical(nullptr,
                tr("Error exporting effect chain preset"),
                tr("Could not save effect chain preset") +
                        QStringLiteral(" \"") + chainPresetName +
                        QStringLiteral("\" ") + tr("to file") +
                        QStringLiteral(" ") + fileName);
        return;
    }

    QDomDocument doc(EffectXml::Chain);
    doc.setContent(EffectXml::FileHeader);
    doc.appendChild(pPreset->toXml(&doc));
    file.write(doc.toString().toUtf8());
    file.close();
}

void EffectChainPresetManager::renamePreset(const QString& oldName) {
    VERIFY_OR_DEBUG_ASSERT(m_effectChainPresets.contains(oldName)) {
        return;
    }

    bool okay = false;
    QString newName = QInputDialog::getText(nullptr,
            tr("Rename effect chain preset"),
            tr("New name for effect chain preset") + QStringLiteral(" \"") +
                    oldName + QStringLiteral("\""),
            QLineEdit::Normal,
            oldName,
            &okay);
    if (!okay) {
        return;
    }

    QString directoryPath = m_pConfig->getSettingsPath() +
            kEffectChainPresetDirectory + kFolderDelimiter;
    QFile file(directoryPath + oldName + kXmlFileExtension);
    if (!file.rename(directoryPath + newName + kXmlFileExtension)) {
        QMessageBox::critical(
                nullptr,
                tr("Could not rename effect chain preset"),
                tr("Could not rename effect chain preset \"%1\"").arg(oldName));
    }

    EffectChainPresetPointer pPreset = m_effectChainPresets.take(oldName);
    int index = m_effectChainPresetsSorted.indexOf(pPreset);
    pPreset->setName(newName);
    m_effectChainPresets.insert(newName, pPreset);
    if (index != -1) {
        m_effectChainPresetsSorted.removeAt(index);
        m_effectChainPresetsSorted.insert(index, pPreset);
    }
    index = m_quickEffectChainPresetsSorted.indexOf(pPreset);
    if (index != -1) {
        m_quickEffectChainPresetsSorted.removeAt(index);
        m_quickEffectChainPresetsSorted.insert(index, pPreset);
    }
    emit effectChainPresetListUpdated();
}

void EffectChainPresetManager::deletePreset(const QString& chainPresetName) {
    VERIFY_OR_DEBUG_ASSERT(m_effectChainPresets.contains(chainPresetName)) {
        return;
    }
    auto pressedButton = QMessageBox::question(nullptr,
            tr("Remove effect chain preset"),
            tr("Are you sure you want to delete the effect chain preset") +
                    QStringLiteral(" \"") + chainPresetName + QStringLiteral("\"?"));
    if (pressedButton != QMessageBox::Yes) {
        return;
    }

    QFile file(m_pConfig->getSettingsPath() + kEffectChainPresetDirectory +
            kFolderDelimiter + chainPresetName + kXmlFileExtension);
    if (!file.remove()) {
        QMessageBox::critical(
                nullptr,
                tr("Could not delete effect chain preset"),
                tr("Could not delete effect chain preset \"%1\"").arg(chainPresetName));
    }

    EffectChainPresetPointer pPreset =
            m_effectChainPresets.take(chainPresetName);
    m_effectChainPresetsSorted.removeAll(pPreset);
    m_quickEffectChainPresetsSorted.removeAll(pPreset);
    emit effectChainPresetListUpdated();
    emit quickEffectChainPresetListUpdated();
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

void EffectChainPresetManager::loadEffectChainPresets() {
    QString savedPresetsPath(
            m_pConfig->getSettingsPath() + kEffectChainPresetDirectory);
    QDir savedPresetsDir(savedPresetsPath);

    // On first run of Mixxx, copy chain presets from the resource folder to the
    // user settings folder. This allows us to ship default chain presets with
    // Mixxx while letting the user delete our defaults if they do not want them.
    // The user can always copy the files again manually or use the Import
    // button in DlgPrefEffects.
    if (!savedPresetsDir.exists()) {
        savedPresetsDir.mkpath(savedPresetsPath);
        QString defaultPresetsPath(
                m_pConfig->getResourcePath() + kEffectChainPresetDirectory);
        QDir defaultChainPresetsDir(defaultPresetsPath);
        defaultChainPresetsDir.setFilter(QDir::Files | QDir::Readable);
        for (const auto& fileName : defaultChainPresetsDir.entryList()) {
            QFile::copy(defaultPresetsPath + kFolderDelimiter + fileName,
                    savedPresetsPath + kFolderDelimiter + fileName);
        }
    }

    savedPresetsDir.setFilter(QDir::Files | QDir::Readable);
    const QStringList fileList = savedPresetsDir.entryList();
    for (const auto& filePath : fileList) {
        QFile file(savedPresetsPath + kFolderDelimiter + filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }
        QDomDocument doc;
        if (!doc.setContent(&file)) {
            file.close();
            continue;
        }
        EffectChainPresetPointer pEffectChainPreset(
                new EffectChainPreset(doc.documentElement()));
        if (!pEffectChainPreset->isEmpty()) {
            m_effectChainPresets.insert(
                    pEffectChainPreset->name(), pEffectChainPreset);
            m_effectChainPresetsSorted.append(pEffectChainPreset);
        }
        file.close();
    }
    emit effectChainPresetListUpdated();
}

void EffectChainPresetManager::savePreset(EffectChainSlotPointer pChainSlot) {
    EffectChainPresetPointer pPreset(new EffectChainPreset(pChainSlot.get()));
    savePreset(pPreset);
}

void EffectChainPresetManager::savePreset(EffectChainPresetPointer pPreset) {
    bool okay = false;
    QString name = QInputDialog::getText(nullptr,
            tr("Save preset for effect chain"),
            tr("Name for new effect chain preset"),
            QLineEdit::Normal,
            pPreset->name(),
            &okay);
    if (!okay) {
        return;
    }

    pPreset->setName(name);
    m_effectChainPresets.insert(name, pPreset);
    m_effectChainPresetsSorted.append(pPreset);
    m_quickEffectChainPresetsSorted.append(pPreset);
    emit effectChainPresetListUpdated();
    emit quickEffectChainPresetListUpdated();

    savePresetXml(pPreset);
}

void EffectChainPresetManager::savePresetXml(EffectChainPresetPointer pPreset) {
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
        return;
    }

    QDomDocument doc(EffectXml::Chain);
    doc.setContent(EffectXml::FileHeader);
    doc.appendChild(pPreset->toXml(&doc));
    file.write(doc.toString().toUtf8());
    file.close();
}

EffectsXmlData EffectChainPresetManager::readEffectsXml(
        const QDomDocument& doc, QStringList deckStrings) {
    DEBUG_ASSERT(!m_effectChainPresets.isEmpty());

    EffectManifestPointer pDefaultQuickEffectManifest = m_pBackendManager->getManifest(
            FilterEffect::getId(), EffectBackendType::BuiltIn);
    auto defaultQuickEffectChainPreset = EffectChainPresetPointer(
            new EffectChainPreset(pDefaultQuickEffectManifest));

    QList<EffectChainPresetPointer> standardEffectChainPresets;
    QHash<QString, EffectChainPresetPointer> quickEffectPresets;
    for (const auto& deckString : deckStrings) {
        quickEffectPresets.insert(deckString, defaultQuickEffectChainPreset);
    }

    // Reload state of standard chains
    QDomElement root = doc.documentElement();
    QDomElement rackElement = XmlParse::selectElement(root, EffectXml::Rack);
    QDomElement chainsElement =
            XmlParse::selectElement(rackElement, EffectXml::ChainsRoot);
    QDomNodeList chainsList = chainsElement.elementsByTagName(EffectXml::Chain);

    for (int i = 0; i < chainsList.count(); ++i) {
        QDomNode chainNode = chainsList.at(i);

        if (chainNode.isElement()) {
            QDomElement chainElement = chainNode.toElement();
            EffectChainPresetPointer pPreset(
                    new EffectChainPreset(chainElement));
            standardEffectChainPresets.append(pPreset);
        }
    }

    // Reload order of custom chain presets
    QStringList chainPresetsSorted;
    QDomElement chainPresetsElement =
            XmlParse::selectElement(root, EffectXml::ChainPresetList);
    QDomNodeList presetNameList =
            chainPresetsElement.elementsByTagName(EffectXml::ChainPresetName);
    for (int i = 0; i < presetNameList.count(); ++i) {
        QDomNode presetNameNode = presetNameList.at(i);
        if (presetNameNode.isElement()) {
            chainPresetsSorted << presetNameNode.toElement().text();
        }
    }
    if (!chainPresetsSorted.isEmpty()) {
        setPresetOrder(chainPresetsSorted);
    }

    // Reload order of QuickEffect chain presets
    QStringList quickEffectChainPresetsSorted;
    QDomElement quickEffectChainPresetsElement =
            XmlParse::selectElement(root, EffectXml::QuickEffectList);
    QDomNodeList quickEffectPresetNameList =
            quickEffectChainPresetsElement.elementsByTagName(EffectXml::ChainPresetName);
    for (int i = 0; i < quickEffectPresetNameList.count(); ++i) {
        QDomNode presetNameNode = quickEffectPresetNameList.at(i);
        if (presetNameNode.isElement()) {
            quickEffectChainPresetsSorted << presetNameNode.toElement().text();
        }
    }
    if (!quickEffectChainPresetsSorted.isEmpty()) {
        setQuickEffectPresetOrder(quickEffectChainPresetsSorted);
    } else {
        // Prepend all chain presets to the QuickEffect preset list
        for (const auto& pChainPreset : m_effectChainPresetsSorted) {
            m_quickEffectChainPresetsSorted.append(pChainPreset);
        }
        // On first run of Mixxx, generate QuickEffect chain presets for every
        // effect with a default metaknob linking. These are generated rather
        // than included with Mixxx in res/effects/chains so the translated
        // names are used.
        QList<EffectManifestPointer> manifestList;
        for (const auto& pManifest : m_pBackendManager->getManifests()) {
            if (pManifest->hasMetaKnobLinking()) {
                manifestList.append(pManifest);
            }
        }
        std::sort(manifestList.begin(), manifestList.end(), EffectManifest::alphabetize);
        for (const auto& pManifest : manifestList) {
            auto pChainPreset = EffectChainPresetPointer(new EffectChainPreset(pManifest));
            pChainPreset->setName(pManifest->displayName());
            m_effectChainPresets.insert(pChainPreset->name(), pChainPreset);
            m_quickEffectChainPresetsSorted.append(pChainPreset);
            savePresetXml(pChainPreset);
        }
        emit quickEffectChainPresetListUpdated();
    }

    // Reload presets that were loaded into QuickEffects on last shutdown
    QDomElement quickEffectPresetsElement =
            XmlParse::selectElement(root, EffectXml::QuickEffectChainPresets);
    QDomNodeList quickEffectNodeList =
            quickEffectPresetsElement.elementsByTagName(
                    EffectXml::ChainPresetName);
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

void EffectChainPresetManager::saveEffectsXml(QDomDocument* pDoc, EffectsXmlData data) {
    // Save presets for current state of standard chains
    QDomElement rootElement = pDoc->documentElement();
    QDomElement rackElement = pDoc->createElement(EffectXml::Rack);
    rootElement.appendChild(rackElement);
    QDomElement chainsElement = pDoc->createElement(EffectXml::ChainsRoot);
    rackElement.appendChild(chainsElement);
    for (const auto& pPreset : data.standardEffectChainPresets) {
        chainsElement.appendChild(pPreset->toXml(pDoc));
    }

    // Save order of custom chain presets
    QDomElement chainPresetListElement =
            pDoc->createElement(EffectXml::ChainPresetList);
    for (const auto& pPreset : m_effectChainPresetsSorted) {
        XmlParse::addElement(*pDoc,
                chainPresetListElement,
                EffectXml::ChainPresetName,
                pPreset->name());
    }
    rootElement.appendChild(chainPresetListElement);

    // Save order of QuickEffect chain presets
    QDomElement quickEffectChainPresetListElement =
            pDoc->createElement(EffectXml::QuickEffectList);
    for (const auto& pPreset : m_quickEffectChainPresetsSorted) {
        XmlParse::addElement(*pDoc,
                quickEffectChainPresetListElement,
                EffectXml::ChainPresetName,
                pPreset->name());
    }
    rootElement.appendChild(quickEffectChainPresetListElement);

    // Save which presets are loaded to QuickEffects
    QDomElement quickEffectPresetsElement =
            pDoc->createElement(EffectXml::QuickEffectChainPresets);
    for (auto it = data.quickEffectChainPresets.begin();
            it != data.quickEffectChainPresets.end();
            it++) {
        QDomElement quickEffectElement = XmlParse::addElement(
                *pDoc,
                quickEffectPresetsElement,
                EffectXml::ChainPresetName,
                it.value()->name());
        quickEffectElement.setAttribute(QStringLiteral("group"), it.key());
    }
    rootElement.appendChild(quickEffectPresetsElement);
}
