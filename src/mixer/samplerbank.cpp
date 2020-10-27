#include "mixer/samplerbank.h"

#include <QFileDialog>
#include <QMessageBox>

#include "control/controlpushbutton.h"
#include "mixer/playermanager.h"
#include "mixer/sampler.h"
#include "track/track.h"
#include "util/assert.h"

SamplerBank::SamplerBank(PlayerManager* pPlayerManager)
        : QObject(pPlayerManager),
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

    m_pCONumSamplers = new ControlProxy(ConfigKey("[Master]", "num_samplers"), this);
}

SamplerBank::~SamplerBank() {
}

void SamplerBank::slotSaveSamplerBank(double v) {
    if (v <= 0.0) {
        return;
    }

    const QString xmlSuffix = QStringLiteral(".xml");
    QString fileFilter = tr("Mixxx Sampler Banks (*%1)").arg(xmlSuffix);
    QString samplerBankPath = QFileDialog::getSaveFileName(nullptr,
            tr("Save Sampler Bank"),
            QString(),
            fileFilter,
            &fileFilter);
    if (samplerBankPath.isNull() || samplerBankPath.isEmpty()) {
        return;
    }

    // Manually add extension due to bug in QFileDialog
    // via https://bugreports.qt-project.org/browse/QTBUG-27186
    QFileInfo fileName(samplerBankPath);
    if (fileName.suffix().isEmpty() || !fileName.suffix().endsWith(xmlSuffix)) {
        samplerBankPath.append(xmlSuffix);
    }

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

    QDomDocument doc("SamplerBank");

    QDomElement root = doc.createElement("samplerbank");
    doc.appendChild(root);

    for (unsigned int i = 0; i < m_pPlayerManager->numSamplers(); ++i) {
        Sampler* pSampler = m_pPlayerManager->getSampler(i + 1);
        if (!pSampler) {
            continue;
        }
        QDomElement samplerNode = doc.createElement(QString("sampler"));

        samplerNode.setAttribute("group", pSampler->getGroup());

        TrackPointer pTrack = pSampler->getLoadedTrack();
        if (pTrack) {
            QString samplerLocation = pTrack->getLocation();
            samplerNode.setAttribute("location", samplerLocation);
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

    QString samplerBankPath = QFileDialog::getOpenFileName(nullptr,
            tr("Load Sampler Bank"),
            QString(),
            tr("Mixxx Sampler Banks (*.xml)"));
    if (samplerBankPath.isEmpty()) {
        return;
    }

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

                if (!group.isEmpty()
                        && m_pPlayerManager->isSamplerGroup(group, &samplerNum)) {
                    if (m_pPlayerManager->numSamplers() < (unsigned) samplerNum) {
                        m_pCONumSamplers->set(samplerNum);
                    }

                    if (location.isEmpty()) {
                        m_pPlayerManager->slotLoadTrackToPlayer(TrackPointer(), group);
                    } else {
                        m_pPlayerManager->slotLoadToPlayer(location, group);
                    }
                }

            }
        }
        n = n.nextSibling();
    }

    file.close();
    return true;
}
