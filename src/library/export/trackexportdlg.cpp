#include "library/export/trackexportdlg.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QMessageBox>

#include "moc_trackexportdlg.cpp"
#include "util/assert.h"

TrackExportDlg::TrackExportDlg(QWidget *parent,
                               UserSettingsPointer pConfig,
                               TrackExportWorker* worker)
        : QDialog(parent),
          Ui::DlgTrackExport(),
          m_pConfig(pConfig),
          m_worker(worker) {
    setupUi(this);
    connect(cancelButton,
            &QPushButton::clicked,
            this,
            &TrackExportDlg::cancelButtonClicked);
    exportProgress->setMinimum(0);
    exportProgress->setMaximum(1);
    exportProgress->setValue(0);
    statusLabel->setText("");
    setModal(true);

    connect(m_worker,
            &TrackExportWorker::progress,
            this,
            &TrackExportDlg::slotProgress);
    connect(m_worker,
            &TrackExportWorker::askOverwriteMode,
            this,
            &TrackExportDlg::slotAskOverwriteMode);
    connect(m_worker,
            &TrackExportWorker::canceled,
            this,
            &TrackExportDlg::cancelButtonClicked);
}

void TrackExportDlg::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    VERIFY_OR_DEBUG_ASSERT(m_worker) {
        // It's not worth checking for m_exporter != nullptr elsewhere in this
        // class... it'll be clear very quickly that someone screwed up and
        // forgot to call selectDestinationDirectory().
        qDebug() << "Programming error: did not initialize m_exporter, about to crash";
        return;
    }
    m_worker->start();
}

void TrackExportDlg::slotProgress(const QString& filename, int progress, int count) {
    if (progress == count) {
        statusLabel->setText(tr("Export finished"));
        finish();
    } else {
        statusLabel->setText(tr("Exporting %1").arg(filename));
    }
    exportProgress->setMinimum(0);
    exportProgress->setMaximum(count);
    exportProgress->setValue(progress);
}

void TrackExportDlg::slotAskOverwriteMode(
        const QString& filename,
        std::promise<TrackExportWorker::OverwriteAnswer>* promise) {
    QMessageBox question_box(
            QMessageBox::Warning,
            tr("Overwrite Existing File?"),
            tr("\"%1\" already exists, overwrite?").arg(filename),
            QMessageBox::Cancel | QMessageBox::No | QMessageBox::NoToAll
            | QMessageBox::Yes | QMessageBox::YesToAll);
    question_box.setDefaultButton(QMessageBox::No);
    question_box.setButtonText(QMessageBox::Yes, tr("&Overwrite"));
    question_box.setButtonText(QMessageBox::YesToAll, tr("Over&write All"));
    question_box.setButtonText(QMessageBox::No, tr("&Skip"));
    question_box.setButtonText(QMessageBox::NoToAll, tr("Skip &All"));

    switch (question_box.exec()) {
    case QMessageBox::No:
        promise->set_value(TrackExportWorker::OverwriteAnswer::SKIP);
        return;
    case QMessageBox::NoToAll:
        promise->set_value(TrackExportWorker::OverwriteAnswer::SKIP_ALL);
        return;
    case QMessageBox::Yes:
        promise->set_value(TrackExportWorker::OverwriteAnswer::OVERWRITE);
        return;
    case QMessageBox::YesToAll:
        promise->set_value(TrackExportWorker::OverwriteAnswer::OVERWRITE_ALL);
        return;
    case QMessageBox::Cancel:
    default:
        promise->set_value(TrackExportWorker::OverwriteAnswer::CANCEL);
    }
}

void TrackExportDlg::cancelButtonClicked() {
    finish();
}

void TrackExportDlg::finish() {
    m_worker->stop();
    m_worker->wait();
    if (m_worker->errorMessage().length()) {
        QMessageBox::warning(
                nullptr,
                tr("Export Error"),
                m_worker->errorMessage(),
                QMessageBox::Ok,
                QMessageBox::Ok);
    }
    hide();
    accept();
}
