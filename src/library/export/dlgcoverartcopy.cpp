#include "library/export/dlgcoverartcopy.h"

#include <QImage>
#include <QMessageBox>

#include "library/coverart.h"
#include "library/library_prefs.h"
#include "util/assert.h"

DlgCoverArtCopy::DlgCoverArtCopy(QWidget* parent,
        UserSettingsPointer pConfig,
        CoverArtCopyWorker* worker)
        : QDialog(parent),
          Ui::CoverArtCopyDlg(),
          m_pConfig(pConfig),
          m_pWCurrentCoverArtLabel(make_parented<WCoverArtLabel>(this)),
          m_pWSelectedCoverArtLabel(make_parented<WCoverArtLabel>(this)),
          m_worker(worker) {
    setupUi(this);
    m_coverOverwritten = false;

    setModal(true);

    currentCoverArtLayout->setAlignment(Qt::AlignRight | Qt::AlignTop);
    currentCoverArtLayout->setSpacing(0);
    currentCoverArtLayout->setContentsMargins(0, 0, 0, 0);
    currentCoverArtLayout->insertWidget(0, m_pWCurrentCoverArtLabel.get());

    selectedCoverArtLayout->setAlignment(Qt::AlignRight | Qt::AlignTop);
    selectedCoverArtLayout->setSpacing(0);
    selectedCoverArtLayout->setContentsMargins(0, 0, 0, 0);
    selectedCoverArtLayout->insertWidget(0, m_pWSelectedCoverArtLabel.get());

    QImage newCoverArt(m_worker->getNewCoverArtImage());
    QImage oldCoverArt(m_worker->getOldCoverArtLocation());

    CoverInfoRelative coverInfo;
    coverInfo.type = CoverInfo::FILE;
    coverInfo.source = CoverInfo::USER_SELECTED;
    coverInfo.coverLocation = m_worker->getOldCoverArtLocation();

    coverInfo.setImage(oldCoverArt);
    int oldCoverArtHeight = oldCoverArt.size().height();
    int oldCoverArtWidth = oldCoverArt.size().width();
    QString oldCoverArtSize = tr("Cover art size: %1x%2");
    labelOldCoverArtSize->setText(
            oldCoverArtSize.arg(QString::number(oldCoverArtWidth),
                    QString::number(oldCoverArtHeight)));
    auto oldCoverArtFileSize = oldCoverArt.sizeInBytes();
    labelOldFileSize->setText(QString::asprintf("File size: %%1").arg(oldCoverArtFileSize));
    m_pWCurrentCoverArtLabel->setCoverArt(CoverInfo{}, QPixmap::fromImage(oldCoverArt));

    coverInfo.setImage(newCoverArt);
    int newCoverArtHeight = newCoverArt.size().height();
    int newCoverArtWidth = newCoverArt.size().width();
    QString newCoverArtSize = tr("Cover art size: %1x%2");
    labelNewCoverArtSize->setText(
            newCoverArtSize.arg(QString::number(newCoverArtWidth),
                    QString::number(newCoverArtHeight)));
    auto newCoverArtFileSize = newCoverArt.sizeInBytes();
    labelNewFileSize->setText(QString::asprintf("File size: %%1").arg(newCoverArtFileSize));

    m_pWSelectedCoverArtLabel->setCoverArt(CoverInfo{}, QPixmap::fromImage(newCoverArt));

    connect(m_worker,
            &CoverArtCopyWorker::askOverwrite,
            this,
            &DlgCoverArtCopy::slotAskOverwrite);
    connect(m_worker,
            &CoverArtCopyWorker::canceled,
            this,
            &DlgCoverArtCopy::btnCancelClicked);
}

void DlgCoverArtCopy::btnCancelClicked() {
    finish();
}

void DlgCoverArtCopy::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    VERIFY_OR_DEBUG_ASSERT(m_worker) {
        qDebug() << "m_worker did not initialized, about to crash";
        return;
    }
    m_worker->start();
}

void DlgCoverArtCopy::slotAskOverwrite(
        const QString& coverArtCopyPath,
        std::promise<CoverArtCopyWorker::OverwriteAnswer>* promise) {
    QFileInfo coverArtInfo(coverArtCopyPath);
    QString coverArtName = coverArtInfo.completeBaseName();
    QString coverArtFolder = coverArtInfo.absolutePath();
    QMessageBox overwrite_box(
            QMessageBox::Warning,
            tr("Cover Art File Already Exists"),
            tr("File: %1\n"
               "Folder: %2\n"
               "Override existing file?\n"
               "This can not be undone!")
                    .arg(coverArtName, coverArtFolder));
    overwrite_box.addButton(QMessageBox::Yes);
    overwrite_box.addButton(QMessageBox::No);

    switch (overwrite_box.exec()) {
    case QMessageBox::No:
        promise->set_value(CoverArtCopyWorker::OverwriteAnswer::UPDATE);
        return;
    case QMessageBox::Yes:
        m_coverOverwritten = true;
        m_coverOverwrittenPath = coverArtCopyPath;
        promise->set_value(CoverArtCopyWorker::OverwriteAnswer::OVERWRITE);
        return;
    }
}

void DlgCoverArtCopy::finish() {
    m_worker->stop();
    m_worker->wait();
    if (m_worker->errorMessage().length()) {
        QMessageBox::warning(
                nullptr,
                tr("Copy Error"),
                m_worker->errorMessage(),
                QMessageBox::Ok,
                QMessageBox::Ok);
    }

    if (m_coverOverwritten &&
            m_pConfig->getValue<bool>(
                    mixxx::library::prefs::kInformCoverArtLocationConfigKey)) {
        QMessageBox::information(nullptr,
                tr("Cover Art Location"),
                tr("Cover Art saved as %1").arg(m_coverOverwrittenPath));
    }

    hide();
    accept();
}
