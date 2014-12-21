#include <QFileDialog>
#include <QMessageBox>

#include "sampler.h"
#include "samplerbank.h"
#include "trackinfoobject.h"
#include "controlpushbutton.h"
#include "playermanager.h"
#include "util/assert.h"

SamplerBank::SamplerBank(PlayerManager* pPlayerManager)
        : QObject(pPlayerManager),
          m_pPlayerManager(pPlayerManager) {
    DEBUG_ASSERT(m_pPlayerManager);
    m_pLoadControl = new ControlPushButton(ConfigKey("[Sampler]", "LoadSamplerBank"));
    connect(m_pLoadControl, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoadSamplerBank(double)));
    m_pSaveControl = new ControlPushButton(ConfigKey("[Sampler]", "SaveSamplerBank"));
    connect(m_pSaveControl, SIGNAL(valueChanged(double)),
            this, SLOT(slotSaveSamplerBank(double)));
}

SamplerBank::~SamplerBank() {
    delete m_pLoadControl;
    delete m_pSaveControl;
}

void SamplerBank::slotSaveSamplerBank(double v) {
    if (v == 0.0 || m_pPlayerManager == NULL) {
        return;
    }

    QString samplerBankPath = QFileDialog::getSaveFileName(
            NULL, tr("Save Sampler Bank"),
            QString(),
            tr("Mixxx Sampler Banks (*.xml)"));
    if (samplerBankPath.isEmpty()) {
        return;
    }

    // The user has picked a new directory via a file dialog. This means the
    // system sandboxer (if we are sandboxed) has granted us permission to this
    // folder. We don't need access to this file on a regular basis so we do not
    // register a security bookmark.

    QFile file(samplerBankPath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(NULL,
                             tr("Error Saving Sampler Bank"),
                             tr("Could not write the sampler bank to '%1'.")
                             .arg(samplerBankPath));
        return;
    }

    QDomDocument doc("SamplerBank");

    QDomElement root = doc.createElement("samplerbank");
    doc.appendChild(root);

    for (unsigned int i = 0; i < m_pPlayerManager->numSamplers(); ++i) {
        Sampler* pSampler = m_pPlayerManager->getSampler(i + 1);
        if (pSampler == NULL) {
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
}

void SamplerBank::slotLoadSamplerBank(double v) {
    if (v == 0.0 || m_pPlayerManager == NULL) {
        return;
    }

    QString samplerBankPath = QFileDialog::getOpenFileName(
            NULL,
            tr("Load Sampler Bank"),
            QString(),
            tr("Mixxx Sampler Banks (*.xml)"));
    if (samplerBankPath.isEmpty()) {
        return;
    }

    // The user has picked a new directory via a file dialog. This means the
    // system sandboxer (if we are sandboxed) has granted us permission to this
    // folder. We don't need access to this file on a regular basis so we do not
    // register a security bookmark.

    QFile file(samplerBankPath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(NULL,
                             tr("Error Reading Sampler Bank"),
                             tr("Could not open the sampler bank file '%1'.")
                             .arg(samplerBankPath));
        return;
    }

    QDomDocument doc;

    if (!doc.setContent(file.readAll())) {
        QMessageBox::warning(NULL,
                             tr("Error Reading Sampler Bank"),
                             tr("Could not read the sampler bank file '%1'.")
                             .arg(samplerBankPath));
        return;
    }

    QDomElement root = doc.documentElement();
    if(root.tagName() != "samplerbank") {
        QMessageBox::warning(NULL,
                             tr("Error Reading Sampler Bank"),
                             tr("Could not read the sampler bank file '%1'.")
                             .arg(samplerBankPath));
        return;
    }

    QDomNode n = root.firstChild();

    while (!n.isNull()) {
        QDomElement e = n.toElement();

        if (!e.isNull()) {
            if (e.tagName() == "sampler") {
                QString group = e.attribute("group", "");
                QString location = e.attribute("location", "");

                if (!group.isEmpty()) {
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
}
