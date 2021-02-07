#include "library/export/trackexportdlg.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QScrollBar>

#include "moc_trackexportdlg.cpp"
#include "util/assert.h"
#include "util/fileutils.h"

namespace {

const QStringList kDefaultPatterns = QStringList()
        << "{{ track.fileName }}"
        << "{{ track.artist }} - {{track.title }}.{{ track.extension }}"
        << "{{ index|zeropad:\"3\"}} - {{ track.artist }} - "
           "{{ track.title }}.{{ track.extension }}"
        << "{{ track.bpm|round }} - {{ track.artist }} - {{track.title }}"
           ".{{ track.extension }}"
        << "{{ track.bpm|round }}/{{ track.artist }} - {{track.title }}"
           ".{{ track.extension }}"
        << "{{ track.bpm|round }}/{{ track.key.openkey }} - {{ track.artist }} "
           "- {{track.title }}.{{ track.extension }}"
        << "{{ track.bpm|round }}/{{ track.key.lancelot }} - {{ track.artist "
           "}} - {{track.title }}.{{ track.extension }}"
        << "{{ track.bpm|rangegroup }}/{{ track.artist }} - {{track.title "
           "}}.{{ track.extension }}"
        << "{{ crate.name }}/{{ index|zeropad:\"3\" }} - {{ track.artist }}"
           " - {{ track.title }}.{{ track.extension }}"
        << "{{ playlist.name }}/{{ index|zeropad:\"3\" }} - {{ track.artist }} - "
           "{{ track.title }}.{{ track.extension }}";

} // anonymous namespace

TrackExportDlg::TrackExportDlg(QWidget* parent,
        UserSettingsPointer pConfig,
        TrackPointerList& tracks,
        Grantlee::Context* context,
        const QString* playlist)
        : QDialog(parent),
          Ui::DlgTrackExport(),
          m_pConfig(pConfig),
          m_tracks(tracks),
          m_worker(nullptr) {
    setupUi(this);

    QString musicDir = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    auto exportDir = QDir(musicDir + "/Mixxx/Export");
    QString lastExportDirectory = m_pConfig->getValue(
            ConfigKey("[Library]", "LastTrackCopyDirectory"),
            exportDir.absolutePath());
    folderEdit->setText(lastExportDirectory);

    m_worker = new TrackExportWorker(folderEdit->text(), m_tracks, context);

    m_patternMenu = new QMenu(patternButton);
    m_patternMenu->installEventFilter(this);

    if (playlist) {
        // use the escaped version as suggestion for playlist name
        playlistName->setText(FileUtils::escapeFileName(*playlist));
    }

    populateDefaultPatterns();

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

    patternEdit->setText(kDefaultPatterns[0]);
    connect(patternEdit,
            &QLineEdit::textChanged,
            [this](const QString& x) {
                Q_UNUSED(x);
                updatePreview();
            });
    patternButton->setMenu(m_patternMenu);

    connect(m_patternMenu,
            &QMenu::triggered,
            this,
            &TrackExportDlg::slotPatternSelected);

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

    updatePreview();
    setEnableControls(true);
    tabWidget->setCurrentIndex(0);
}

TrackExportDlg::~TrackExportDlg() {
}

void TrackExportDlg::populateDefaultPatterns() {
    for (const auto& pattern : kDefaultPatterns) {
        m_patternMenu->addAction(pattern);
    }
}

void TrackExportDlg::slotPatternSelected(QAction* action) {
    patternEdit->setText(action->text());
}

bool TrackExportDlg::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::Show && obj == m_patternMenu) {
        QPoint pos = m_patternMenu->pos();
        pos.rx() -= m_patternMenu->width() - patternButton->width();
        m_patternMenu->move(pos);
        return true;
    }
    return false;
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
    patternEdit->setEnabled(enabled);
    folderEdit->setEnabled(enabled);
    browseButton->setEnabled(enabled);
    playlistName->setEnabled(enabled);
    playlistExport->setEnabled(enabled);
    playlistSuffix->setEnabled(enabled);
    patternButton->setEnabled(enabled);
}

void TrackExportDlg::slotStartExport() {
    VERIFY_OR_DEBUG_ASSERT(m_worker->isRunning() == false) {
        qWarning() << "Export already running";
        return;
    }

    m_errorCount = 0;
    m_okCount = 0;
    m_skippedCount = 0;

    m_pConfig->setValue(
            ConfigKey("[Library]", "LastTrackCopyDirectory"),
            folderEdit->text());

    // sets destDirectory and Pattern
    updatePreview();
    setEnableControls(false);
    tabWidget->setCurrentIndex(1);

    cancelButton->setText(tr("&Cancel"));

    // enable playlist export
    if (playlistExport->isChecked()) {
        m_worker->setPlaylist(playlistName->text() + playlistSuffix->currentText());
    } else {
        m_worker->setPlaylist(QString());
    }

    m_worker->start();
}

void TrackExportDlg::updatePreview() {
    VERIFY_OR_DEBUG_ASSERT(!m_tracks.isEmpty()) {
        return;
    }
    QString pattern = patternEdit->text();
    m_worker->setPattern(&pattern);
    m_worker->setDestDir(folderEdit->text());
    previewLabel->setText(m_worker->applyPattern(m_tracks[0], 1));
    errorLabel->setText(m_worker->errorMessage());
}

int TrackExportDlg::addStatus(const QString& status, const QString& to) {
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

void TrackExportDlg::slotResult(TrackExportWorker::ExportResult result, const QString& msg) {
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

void TrackExportDlg::slotProgress(const QString& from, const QString& to, int progress, int count) {
    if (!from.isEmpty() || !to.isEmpty()) {
        addStatus(from, to);
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

void TrackExportDlg::open() {
    bool empty = true;
    // check if at least one trackpointer is valid
    for (const TrackPointer& track : qAsConst(m_tracks)) {
        if (track) {
            empty = false;
            break;
        }
    }

    if (empty) {
        QMessageBox::warning(
                nullptr,
                tr("Export Error"),
                tr("No files selected"),
                QMessageBox::Ok,
                QMessageBox::Ok);
        hide();
        accept();
        return;
    }
    QDialog::open();
}

void TrackExportDlg::finish() {
    stopWorker();
}
