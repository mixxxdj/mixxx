#include "library/export/dlgcoverartcopy.h"

#include <QMessageBox>

#include "library/library_prefs.h"
#include "util/assert.h"

DlgCoverArtCopy::DlgCoverArtCopy(QWidget* parent,
        UserSettingsPointer pConfig,
        CoverArtCopyWorker* worker)
        : QDialog(parent),
          Ui::CoverArtCopyDlg(),
          m_pConfig(pConfig),
          m_worker(worker) {
    setupUi(this);
    m_coverOverwritten = false;

    setModal(true);

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
    QMessageBox overwrite_box(
            QMessageBox::Warning,
            tr("Overwrite Existing Cover Art?"),
            tr("\"%1\" already exists, overwrite?").arg(coverArtCopyPath));
    overwrite_box.addButton(QMessageBox::Yes)->setText(tr("&Overwrite"));
    overwrite_box.addButton(QMessageBox::No)->setText(tr("&Update without Overwrite"));

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
