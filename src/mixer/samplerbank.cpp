#include "mixer/samplerbank.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>

#include "control/controlpushbutton.h"
#include "mixer/playermanager.h"
#include "mixer/sampler.h"
#include "moc_samplerbank.cpp"
#include "track/track.h"
#include "util/assert.h"
#include "util/file.h"

namespace {

const ConfigKey kConfigkeyLastImportExportDirectory(
        "[Samplers]", "last_import_export_directory");
// This is used in multiple tr() calls below which accepts const char* as a key.
// lupdate finds the single string here.
const char kSamplerFileType[] = QT_TRANSLATE_NOOP("SamplerBank", "Mixxx Sampler Banks (*.xml)");

} // anonymous namespace

SamplerBank::SamplerBank(UserSettingsPointer pConfig,
        PlayerManager* pPlayerManager)
        : QObject(pPlayerManager),
          m_pConfig(pConfig),
          m_pPlayerManager(pPlayerManager) {
    DEBUG_ASSERT(m_pPlayerManager);

    m_pCOLoadBank = std::make_unique<ControlPushButton>(ConfigKey("[Sampler]", "LoadSamplerBank"), this);
    connect(m_pCOLoadBank.get(),
            &ControlObject::valueChanged,
            this,
            &SamplerBank::slotLoadSamplerBank);

    m_pCOSaveBank = std::make_unique<ControlPushButton>(ConfigKey("[Sampler]", "SaveSamplerBank"), this);
    connect(m_pCOSaveBank.get(),
            &ControlObject::valueChanged,
            this,
            &SamplerBank::slotSaveSamplerBank);

    m_pCONumSamplers = new ControlProxy(
            ConfigKey(QStringLiteral("[App]"), QStringLiteral("num_samplers")),
            this);
}

void SamplerBank::slotSaveSamplerBank(double v) {
    if (v <= 0.0) {
        return;
    }

    QString lastImportExportDirectory = m_pConfig->getValue(
            kConfigkeyLastImportExportDirectory,
            // When Mixxx exits samplers are auto-exported to the config directory.
            // Let's choose a different location to avoid confusion.
            QStandardPaths::writableLocation(QStandardPaths::MusicLocation));

    // Open a dialog to let the user choose the file location for crate export.
    // The location is set to the last used directory for import/export and the file
    // name to the playlist name.
    const QString samplerBankPath = getFilePathWithVerifiedExtensionFromFileDialog(
            tr("Save Sampler Bank"),
            lastImportExportDirectory.append("/").append("samplers").append(".xml"),
            tr(kSamplerFileType),
            tr(kSamplerFileType));
    // Exit method if user cancelled the open dialog.
    if (samplerBankPath.isEmpty()) {
        return;
    }

    // Update the import/export directory
    QString fileDirectory(samplerBankPath);
    fileDirectory.truncate(samplerBankPath.lastIndexOf("/"));
    m_pConfig->set(kConfigkeyLastImportExportDirectory,
            ConfigValue(fileDirectory));

    if (!saveSamplerBankToPath(samplerBankPath)) {
        QMessageBox::warning(nullptr,
                tr("Error Saving Sampler Bank"),
                tr("Could not write the sampler bank to '%1'.")
                        .arg(samplerBankPath));
    }
}

bool SamplerBank::saveSamplerBankToPath(const QString& samplerBankPath) {
    // The user has picked a new directory via a file dialog. This means the
    // system sandboxer (if we are sandboxed) has granted us permission to this
    // folder. We don't need access to this file on a regular basis so we do not
    // register a security bookmark.

    VERIFY_OR_DEBUG_ASSERT(m_pPlayerManager) {
        qWarning() << "SamplerBank::saveSamplerBankToPath called with no PlayerManager";
        return false;
    }

    QFile file(samplerBankPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Error saving sampler bank: Could not write to file"
                   << samplerBankPath;
        return false;
    }

    QDomDocument doc(QStringLiteral("SamplerBank"));

    QDomElement root = doc.createElement(QStringLiteral("samplerbank"));
    doc.appendChild(root);

    for (unsigned int i = 0; i < m_pPlayerManager->numSamplers(); ++i) {
        Sampler* pSampler = m_pPlayerManager->getSampler(i + 1);
        if (!pSampler) {
            continue;
        }
        QDomElement samplerNode = doc.createElement(QStringLiteral("sampler"));

        samplerNode.setAttribute(QStringLiteral("group"), pSampler->getGroup());

        TrackPointer pTrack = pSampler->getLoadedTrack();
        if (pTrack) {
            QString samplerLocation = pTrack->getLocation();
            samplerNode.setAttribute(QStringLiteral("location"), samplerLocation);
        }
        root.appendChild(samplerNode);
    }

    QString docStr = doc.toString();

    file.write(docStr.toUtf8().constData());
    file.close();

    return true;
}

void SamplerBank::slotLoadSamplerBank(double v) {
    if (v <= 0.0) {
        return;
    }

    QString lastImportExportDirectory = m_pConfig->getValue(
            kConfigkeyLastImportExportDirectory,
            QStandardPaths::writableLocation(QStandardPaths::MusicLocation));

    QString fileFilter(tr(kSamplerFileType));
    QString samplerBankPath = QFileDialog::getOpenFileName(nullptr,
            tr("Load Sampler Bank"),
            lastImportExportDirectory,
            fileFilter,
            &fileFilter);
    if (samplerBankPath.isEmpty()) {
        return;
    }

    // Update the import/export directory
    QString fileDirectory(samplerBankPath);
    fileDirectory.truncate(samplerBankPath.lastIndexOf("/"));
    m_pConfig->set(kConfigkeyLastImportExportDirectory,
            ConfigValue(fileDirectory));

    if (!loadSamplerBankFromPath(samplerBankPath)) {
        QMessageBox::warning(nullptr,
                tr("Error Reading Sampler Bank"),
                tr("Could not open the sampler bank file '%1'.")
                        .arg(samplerBankPath));
    }
}

bool SamplerBank::loadSamplerBankFromPath(const QString& samplerBankPath) {
    // The user has picked a new directory via a file dialog. This means the
    // system sandboxer (if we are sandboxed) has granted us permission to this
    // folder. We don't need access to this file on a regular basis so we do not
    // register a security bookmark.

    VERIFY_OR_DEBUG_ASSERT(m_pPlayerManager) {
        qWarning() << "SamplerBank::loadSamplerBankFromPath called with no PlayerManager";
        return false;
    }

    QFile file(samplerBankPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not read sampler bank file" << samplerBankPath;
        return false;
    }

    QDomDocument doc;

    if (!doc.setContent(file.readAll())) {
        qWarning() << "Could not read sampler bank file" << samplerBankPath;
        return false;
    }

    QDomElement root = doc.documentElement();
    if (root.tagName() != "samplerbank") {
        qWarning() << "Could not read sampler bank file" << samplerBankPath;
        return false;
    }

    QDomNode n = root.firstChild();

    while (!n.isNull()) {
        QDomElement e = n.toElement();

        if (!e.isNull()) {
            if (e.tagName() == "sampler") {
                QString group = e.attribute("group", "");
                QString location = e.attribute("location", "");
                int samplerNum;

                if (!group.isEmpty() && m_pPlayerManager->isSamplerGroup(group, &samplerNum)) {
                    if (m_pPlayerManager->numSamplers() < (unsigned)samplerNum) {
                        m_pCONumSamplers->set(samplerNum);
                    }

                    if (location.isEmpty()) {
                        m_pPlayerManager->slotLoadTrackToPlayer(
                                TrackPointer(), group,
#ifdef __STEM__
                                mixxx::StemChannelSelection(),
#endif
                                false);
                    } else {
                        m_pPlayerManager->slotLoadLocationToPlayer(location, group, false);
                    }
                }
            }
        }
        n = n.nextSibling();
    }

    file.close();
    return true;
}
