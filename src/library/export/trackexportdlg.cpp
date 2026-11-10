#include "library/export/trackexportdlg.h"

#include <QCheckBox>
#include <QMessageBox>

#include "moc_trackexportdlg.cpp"
#include "util/assert.h"

TrackExportDlg::TrackExportDlg(QWidget* parent,
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
        qDebug() << "Programming error: did not initialize m_exporter, about to crash";
        return;
    }
    m_worker->start();
}

void TrackExportDlg::slotProgress(const QString& filename, int progress, int count) {
    if (progress == count) {
        statusLabel->setText(tr("Export finished"));
        finish(true);
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
            tr("Replace Existing File?"),
            tr("\"%1\" already exists, replace?").arg(filename),
            QMessageBox::Cancel,
            this);

    QPushButton* pSkip = question_box.addButton(
            tr("&Skip"), QMessageBox::NoRole);
    QPushButton* pOverwrite = question_box.addButton(
            tr("&Replace"), QMessageBox::YesRole);
    question_box.setDefaultButton(pSkip);

    QCheckBox* pApplyToAll = new QCheckBox(tr("Apply to all files"));
    pApplyToAll->setChecked(false);
    question_box.setCheckBox(pApplyToAll);

    question_box.exec();
    auto* pBtn = question_box.clickedButton();
    if (pBtn == pSkip) {
        promise->set_value(pApplyToAll->isChecked()
                        ? TrackExportWorker::OverwriteAnswer::SKIP_ALL
                        : TrackExportWorker::OverwriteAnswer::SKIP);
    } else if (pBtn == pOverwrite) {
        promise->set_value(pApplyToAll->isChecked()
                        ? TrackExportWorker::OverwriteAnswer::OVERWRITE_ALL
                        : TrackExportWorker::OverwriteAnswer::OVERWRITE);
    } else {
        // Cancel
        promise->set_value(TrackExportWorker::OverwriteAnswer::CANCEL);
    }
}

void TrackExportDlg::cancelButtonClicked() {
    finish(false);
}

void TrackExportDlg::finish(bool completed) {
    if (m_finished) {
        return;
    }
    m_finished = true;

    m_worker->stop();
    m_worker->wait();

    if (!m_worker->errorMessage().isEmpty()) {
        QMessageBox::warning(
                this,
                tr("Export Error"),
                m_worker->errorMessage(),
                QMessageBox::Ok,
                QMessageBox::Ok);
    } else if (completed && m_worker->failedCount() > 0) {
        const QString summaryText = tr(
                "Export completed, but %1 file(s) could not be exported "
                "because they were missing or inaccessible.")
                                            .arg(m_worker->failedCount());

        QMessageBox warningBox(
                QMessageBox::Warning,
                tr("Export Summary"),
                summaryText,
                QMessageBox::Ok,
                this);
        warningBox.setDetailedText(m_worker->failedTracks().join("\n"));
        warningBox.exec();
    }

    hide();
    accept();
}
