#include "library/export/trackexportdlg.h"

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
            QMessageBox::Cancel);

    QPushButton* pSkip = question_box.addButton(
            tr("&Skip"), QMessageBox::NoRole);
    QPushButton* pSkipAll = question_box.addButton(
            tr("Skip &All"), QMessageBox::NoRole);
    QPushButton* pOverwrite = question_box.addButton(
            tr("&Overwrite"), QMessageBox::YesRole);
    QPushButton* pOverwriteAll = question_box.addButton(
            tr("Over&write All"), QMessageBox::YesRole);
    question_box.setDefaultButton(pSkip);

    question_box.exec();
    auto* pBtn = question_box.clickedButton();
    if (pBtn == pSkip) {
        promise->set_value(TrackExportWorker::OverwriteAnswer::SKIP);
    } else if (pBtn == pSkipAll) {
        promise->set_value(TrackExportWorker::OverwriteAnswer::SKIP_ALL);
    } else if (pBtn == pOverwrite) {
        promise->set_value(TrackExportWorker::OverwriteAnswer::OVERWRITE);
    } else if (pBtn == pOverwriteAll) {
        promise->set_value(TrackExportWorker::OverwriteAnswer::OVERWRITE_ALL);
    } else {
        // Cancel
        promise->set_value(TrackExportWorker::OverwriteAnswer::CANCEL);
    }
}

void TrackExportDlg::cancelButtonClicked() {
    finish();
}

void TrackExportDlg::finish() {
    m_worker->stop();
    m_worker->wait();
    if (!m_worker->errorMessage().isEmpty()) {
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
