#include "library/export/trackexportdlg.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QScrollBar>

#include "moc_trackexportdlg.cpp"
#include "util/assert.h"

TrackExportDlg::TrackExportDlg(QWidget* parent,
        UserSettingsPointer pConfig,
        TrackPointerList& tracks,
        Grantlee::Context* context)
        : QDialog(parent),
          Ui::DlgTrackExport(),
          m_pConfig(pConfig),
          m_tracks(tracks),
          m_worker(nullptr),
          m_context(context) {
    setupUi(this);

    QString lastExportDirectory = m_pConfig->getValue(
            ConfigKey("[Library]", "LastTrackCopyDirectory"),
            QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
    folderEdit->setText(lastExportDirectory);

    m_worker = new TrackExportWorker(folderEdit->text(), m_tracks);

    connect(cancelButton,
            &QPushButton::clicked,
            this,
            &TrackExportDlg::cancelButtonClicked);
    connect(startButton,
            &QPushButton::clicked,
            this,
            &TrackExportDlg::slotStartExport);
    exportProgress->setMinimum(0);
    exportProgress->setMaximum(1);
    exportProgress->setValue(0);
    setModal(true);

    QHeaderView* headerView = statusTable->horizontalHeader();
    headerView->resizeSection(0, 600);
    headerView->resizeSection(1, 600);
    headerView->resizeSection(2, 100);
    headerView->setStretchLastSection(true);
    statusTable->setHorizontalHeader(headerView);

    connect(browseButton,
            &QAbstractButton::clicked,
            this,
            &TrackExportDlg::slotBrowseFolder);
    connect(folderEdit,
            &QLineEdit::textChanged,
            [=](const QString& x) {
                Q_UNUSED(x);
                updatePreview();
            });
    connect(comboPattern,
            &QComboBox::currentTextChanged,
            [this](const QString& x) {
                Q_UNUSED(x);
                updatePreview();
            });

    connect(m_worker,
            &TrackExportWorker::progress,
            this,
            &TrackExportDlg::slotProgress);
    connect(m_worker,
            &TrackExportWorker::result,
            this,
            &TrackExportDlg::slotResult);
    connect(m_worker,
            &TrackExportWorker::askOverwriteMode,
            this,
            &TrackExportDlg::slotAskOverwriteMode);
    connect(m_worker,
            &TrackExportWorker::canceled,
            this,
            &TrackExportDlg::stopWorker);

    if (m_tracks.isEmpty()) {
        QMessageBox::warning(
                nullptr,
                tr("Export Error"),
                tr("No files selected"),
                QMessageBox::Ok,
                QMessageBox::Ok);
        hide();
        accept();
    }
    updatePreview();
}

TrackExportDlg::~TrackExportDlg() {
    if (m_context) {
        delete m_context;
    }
}

bool TrackExportDlg::browseFolder() {
    QString destDir = QFileDialog::getExistingDirectory(
            nullptr, tr("Export Track Files To"), folderEdit->text());
    if (destDir.isEmpty()) {
        return false;
    }
    folderEdit->setText(destDir);
    return true;
}
void TrackExportDlg::closeEvent(QCloseEvent* event) {
    if (m_worker->isRunning()) {
        stopWorker();
    }
    QDialog::closeEvent(event);
}

void TrackExportDlg::setEnableControls(bool enabled) {
    startButton->setEnabled(enabled);
    comboPattern->setEnabled(enabled);
    folderEdit->setEnabled(enabled);
    browseButton->setEnabled(enabled);
}

void TrackExportDlg::slotStartExport() {
    VERIFY_OR_DEBUG_ASSERT(m_worker->isRunning() == false) {
        qWarning() << "Export already running";
        return;
    }
    // Do we want to check for target folder to exist ?
    // When I file is exported, all parent folders are created automatically.

    // QString destDir = folderEdit->text();
    // if (!QFile::exists(destDir)) {
    //     if (!browseFolder()) {
    //         return;
    //     }
    //     destDir = folderEdit->text();
    // }
    m_errorCount = 0;
    m_okCount = 0;
    m_skippedCount = 0;

    m_pConfig->setValue(
            ConfigKey("[Library]", "LastTrackCopyDirectory"),
            folderEdit->text());

    // sets destDirectory and Pattern
    updatePreview();
    setEnableControls(false);
    cancelButton->setText(tr("&Cancel"));

    m_worker->start();
}

void TrackExportDlg::updatePreview() {
    VERIFY_OR_DEBUG_ASSERT(!m_tracks.isEmpty()) {
        return;
    }
    QString pattern = comboPattern->currentText();
    m_worker->setPattern(&pattern);
    m_worker->setDestDir(folderEdit->text());
    previewLabel->setText(m_worker->applyPattern(m_tracks[0], 0));
    errorLabel->setText(m_worker->errorMessage());
}

int TrackExportDlg::addStatus(const QString status, const QString to) {
    auto scrollbar = statusTable->verticalScrollBar();
    bool atEnd = scrollbar->value() == scrollbar->maximum();
    int row = statusTable->rowCount();
    statusTable->setRowCount(row + 1);
    auto item = new QTableWidgetItem(status);
    statusTable->setItem(row, 0, item);
    if (atEnd) {
        statusTable->scrollToBottom();
    }
    if (!to.isEmpty()) {
        auto toItem = new QTableWidgetItem(to);
        statusTable->setItem(row, 1, toItem);
    }
    return row;
}

void TrackExportDlg::slotResult(TrackExportWorker::ExportResult result, const QString msg) {
    int type = QTableWidgetItem::UserType;
    if (result == TrackExportWorker::ExportResult::EXPORT_COMPLETE) {
        exportProgress->setValue(exportProgress->maximum());
        addStatus(QString(tr("Export finished. %1 ok. %2 errors. %3 skipped"))
                          .arg(m_okCount)
                          .arg(m_errorCount)
                          .arg(m_skippedCount),
                nullptr);
        finish();
        return;
    } else if (result == TrackExportWorker::ExportResult::OK) {
        m_okCount++;
    } else if (result == TrackExportWorker::ExportResult::SKIPPED) {
        m_skippedCount++;
        type = QTableWidgetItem::UserType + 2;
    } else if (result == TrackExportWorker::ExportResult::FAILED) {
        m_errorCount++;
        type = QTableWidgetItem::UserType + 3;
    }
    auto item = new QTableWidgetItem(msg, type);
    int row = statusTable->rowCount() - 1;
    statusTable->setItem(row, 2, item);
}

void TrackExportDlg::slotProgress(const QString from, const QString to, int progress, int count) {
    addStatus(from, to);
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
    // we cancel the worker if running, otherwise we close the window
    if (m_worker->isRunning()) {
        stopWorker();
    } else {
        hide();
        accept();
    }
}

void TrackExportDlg::stopWorker() {
    m_worker->stop();
    m_worker->wait();
    setEnableControls(true);
    cancelButton->setText(tr("&Close"));
}

void TrackExportDlg::finish() {
    stopWorker();
}
