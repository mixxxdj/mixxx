#include "effects/presets/effectchainpresetmanager.h"

#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

#include "effects/builtin/filtereffect.h"
#include "effects/effectsmanager.h"
#include "effects/effectxmlelements.h"

namespace {
const QString kEffectChainPresetDirectory = "/effects/chains";
const QString kEffectsXmlFile = "effects.xml";
} // anonymous namespace

EffectChainPresetManager::EffectChainPresetManager(UserSettingsPointer pConfig,
        EffectsBackendManagerPointer pBackendManager)
        : m_pConfig(pConfig),
          m_pBackendManager(pBackendManager) {
    // Generate the default QuickEffect chain preset instead of shipping it in
    // res/effects/chains because this uses the translated name.
    EffectManifestPointer pDefaultQuickEffectManifest = m_pBackendManager->getManifest(
            FilterEffect::getId(), EffectBackendType::BuiltIn);
    m_pDefaultQuickEffectChainPreset = EffectChainPresetPointer(
            new EffectChainPreset(EffectPresetPointer(
                    new EffectPreset(pDefaultQuickEffectManifest))));
    m_pDefaultQuickEffectChainPreset->setName(
            pDefaultQuickEffectManifest->displayName());

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

void EffectChainPresetManager::importPreset() {
    QStringList fileNames = QFileDialog::getOpenFileNames(nullptr,
            tr("Import effect chain preset"),
            QString(),
            tr("Mixxx Effect Chain Presets") + " (*.xml)");

    QString importFailed = tr("Error importing effect chain preset");
    for (int i = 0; i < fileNames.size(); ++i) {
        QString filePath = fileNames.at(i);
        QDomDocument doc;
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(
                    nullptr, importFailed, importFailed + " " + filePath);
            continue;
        } else if (!doc.setContent(&file)) {
            file.close();
            QMessageBox::critical(
                    nullptr, importFailed, importFailed + " " + filePath);
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
                        tr("An effect chain preset with the name") + " " +
                                pPreset->name() + " " +
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
            for (const auto pEffectPreset : pPreset->effectPresets()) {
                if (pEffectPreset == nullptr || pEffectPreset->isEmpty()) {
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
                            tr("The effect chain imported from") + " " +
                                    filePath + " " +
                                    tr("contains an effect that Mixxx does not "
                                       "support") +
                                    ":\n\n" + pEffectPreset->id() + "\n\n" +
                                    tr("If you load this chain preset, the "
                                       "unsupported effect will not be loaded "
                                       "with it."));
                }
            }

            m_effectChainPresets.insert(pPreset->name(), pPreset);
            m_effectChainPresetsSorted.append(pPreset);
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

    QString fileName = QFileDialog::getSaveFileName(nullptr,
            tr("Save effect chain preset"),
            QString(),
            tr("Mixxx Effect Chain Presets") + " (*.xml)");

    QFile file(fileName);
    if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        file.close();
        QMessageBox::critical(nullptr,
                tr("Error exporting effect chain preset"),
                tr("Could not save effect chain preset") + " " +
                        chainPresetName + tr("to file") + " " + fileName);
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
            tr("New name effect chain preset") + " " + oldName,
            QLineEdit::Normal,
            oldName,
            &okay);
    if (!okay) {
        return;
    }

    EffectChainPresetPointer pPreset = m_effectChainPresets.take(oldName);
    int index = m_effectChainPresetsSorted.indexOf(pPreset);
    pPreset->setName(newName);
    m_effectChainPresets.insert(newName, pPreset);
    m_effectChainPresetsSorted.removeAt(index);
    m_effectChainPresetsSorted.insert(index, pPreset);
    emit effectChainPresetListUpdated();
}

void EffectChainPresetManager::deletePreset(const QString& chainPresetName) {
    VERIFY_OR_DEBUG_ASSERT(m_effectChainPresets.contains(chainPresetName)) {
        return;
    }
    auto pressedButton = QMessageBox::question(nullptr,
            tr("Remove effect chain preset"),
            tr("Are you sure you want to delete the effect chain preset") +
                    " " + chainPresetName + "?");
    if (pressedButton != QMessageBox::Yes) {
        return;
    }

    EffectChainPresetPointer pPreset =
            m_effectChainPresets.take(chainPresetName);
    m_effectChainPresetsSorted.removeAll(pPreset);
    emit effectChainPresetListUpdated();
}

void EffectChainPresetManager::setPresetOrder(
        const QStringList& chainPresetList) {
    m_effectChainPresetsSorted.clear();

    for (const auto chainPresetName : chainPresetList) {
        VERIFY_OR_DEBUG_ASSERT(m_effectChainPresets.contains(chainPresetName)) {
            continue;
        }
        m_effectChainPresetsSorted.append(
                m_effectChainPresets.value(chainPresetName));
    }

    for (const auto pPreset : m_effectChainPresets) {
        VERIFY_OR_DEBUG_ASSERT(m_effectChainPresetsSorted.contains(pPreset)) {
            m_effectChainPresetsSorted.append(pPreset);
        }
    }
    emit effectChainPresetListUpdated();
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
            QFile::copy(defaultPresetsPath + "/" + fileName,
                    savedPresetsPath + "/" + fileName);
        }

        QFile file(savedPresetsPath + "/" +
                m_pDefaultQuickEffectChainPreset->name() + ".xml");
        if (file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
            QDomDocument doc(EffectXml::Chain);
            doc.setContent(EffectXml::FileHeader);
            doc.appendChild(m_pDefaultQuickEffectChainPreset->toXml(&doc));
            file.write(doc.toString().toUtf8());
        }
        file.close();
    }

    savedPresetsDir.setFilter(QDir::Files | QDir::Readable);
    QStringList fileList = savedPresetsDir.entryList();
    for (const auto& filePath : fileList) {
        QFile file(savedPresetsPath + "/" + filePath);
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
    emit effectChainPresetListUpdated();

    QString path(m_pConfig->getSettingsPath() + kEffectChainPresetDirectory);
    QDir effectsChainsDir(path);
    if (!effectsChainsDir.exists()) {
        effectsChainsDir.mkpath(path);
    }
    // TODO: sanitize file name?
    QFile file(path + "/" + name + ".xml");
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
        QStringList deckStrings) {
    DEBUG_ASSERT(!m_effectChainPresets.isEmpty());

    QList<EffectChainPresetPointer> standardEffectChainPresets;
    QHash<QString, EffectChainPresetPointer> quickEffectPresets;
    for (const auto& deckString : deckStrings) {
        quickEffectPresets.insert(deckString, m_pDefaultQuickEffectChainPreset);
    }

    QDir settingsPath(m_pConfig->getSettingsPath());
    QFile file(settingsPath.absoluteFilePath(kEffectsXmlFile));
    QDomDocument doc;

    if (!file.open(QIODevice::ReadOnly)) {
        file.close();
        return EffectsXmlData{quickEffectPresets, standardEffectChainPresets};
    } else if (!doc.setContent(&file)) {
        file.close();
        return EffectsXmlData{quickEffectPresets, standardEffectChainPresets};
    }
    file.close();

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
    setPresetOrder(chainPresetsSorted);

    // Load QuickEffect presets
    QDomElement quickEffectPresetsElement =
            XmlParse::selectElement(root, EffectXml::QuickEffectChainPresets);
    QDomNodeList quickEffectNodeList =
            quickEffectPresetsElement.elementsByTagName(
                    EffectXml::ChainPresetName);
    for (int i = 0; i < quickEffectNodeList.count(); ++i) {
        QDomElement presetNameElement = quickEffectNodeList.at(i).toElement();
        if (!presetNameElement.isNull()) {
            QString deckGroup = presetNameElement.attribute("group");
            auto pPreset = m_effectChainPresets.value(presetNameElement.text());
            if (pPreset != nullptr) {
                quickEffectPresets.insert(deckGroup, pPreset);
            }
        }
    }

    return EffectsXmlData{quickEffectPresets, standardEffectChainPresets};
}

void EffectChainPresetManager::saveEffectsXml(EffectsXmlData data) {
    QDomDocument doc(EffectXml::Root);
    doc.setContent(EffectXml::FileHeader);

    // Save presets for current state of standard chains
    QDomElement rootElement = doc.createElement(EffectXml::Root);
    rootElement.setAttribute(
            "schemaVersion", QString::number(EffectXml::kXmlSchemaVersion));
    doc.appendChild(rootElement);
    QDomElement rackElement = doc.createElement(EffectXml::Rack);
    rootElement.appendChild(rackElement);
    QDomElement chainsElement = doc.createElement(EffectXml::ChainsRoot);
    rackElement.appendChild(chainsElement);
    for (const auto pPreset : data.standardEffectChainPresets) {
        chainsElement.appendChild(pPreset->toXml(&doc));
    }

    // Save order of custom chain presets
    QDomElement chainPresetListElement =
            doc.createElement(EffectXml::ChainPresetList);
    for (const auto pPreset : m_effectChainPresetsSorted) {
        XmlParse::addElement(doc,
                chainPresetListElement,
                EffectXml::ChainPresetName,
                pPreset->name());
    }
    rootElement.appendChild(chainPresetListElement);

    // Save which presets are loaded to QuickEffects
    QDomElement quickEffectPresetsElement =
            doc.createElement(EffectXml::QuickEffectChainPresets);
    for (auto it = data.quickEffectChainPresets.begin();
            it != data.quickEffectChainPresets.end();
            it++) {
        QDomElement quickEffectElement = XmlParse::addElement(
                doc,
                quickEffectPresetsElement,
                EffectXml::ChainPresetName,
                it.value()->name());
        quickEffectElement.setAttribute("group", it.key());
    }
    rootElement.appendChild(quickEffectPresetsElement);

    QDir settingsPath(m_pConfig->getSettingsPath());
    if (!settingsPath.exists()) {
        return;
    }
    QFile file(settingsPath.absoluteFilePath(kEffectsXmlFile));
    if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        return;
    }
    file.write(doc.toString().toUtf8());
    file.close();
}
