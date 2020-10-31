#include <QDesktopServices>
#include <QStandardPaths>
#include <QDir>
#include <QFileDialog>
#include <QStringList>
#include <QUrl>
#include <QApplication>
#include <QFontDialog>
#include <QFontMetrics>
#include <QMessageBox>

#include "preferences/dialog/dlgpreflibrary.h"
#include "library/dlgtrackmetadataexport.h"
#include "sources/soundsourceproxy.h"
#include "widget/wsearchlineedit.h"

namespace {
    const ConfigKey kSearchDebouncingTimeoutMillisKey = ConfigKey("[Library]","SearchDebouncingTimeoutMillis");
}

DlgPrefLibrary::DlgPrefLibrary(
        QWidget* pParent,
        UserSettingsPointer pConfig,
        Library* pLibrary)
        : DlgPreferencePage(pParent),
          m_dirListModel(),
          m_pConfig(pConfig),
          m_pLibrary(pLibrary),
          m_bAddedDirectory(false),
          m_iOriginalTrackTableRowHeight(Library::kDefaultRowHeightPx) {
    setupUi(this);

    connect(this,
            &DlgPrefLibrary::requestAddDir,
            m_pLibrary,
            &Library::slotRequestAddDir);
    connect(this,
            &DlgPrefLibrary::requestRemoveDir,
            m_pLibrary,
            &Library::slotRequestRemoveDir);
    connect(this,
            &DlgPrefLibrary::requestRelocateDir,
            m_pLibrary,
            &Library::slotRequestRelocateDir);
    connect(PushButtonAddDir,
            &QPushButton::clicked,
            this,
            &DlgPrefLibrary::slotAddDir);
    connect(PushButtonRemoveDir,
            &QPushButton::clicked,
            this,
            &DlgPrefLibrary::slotRemoveDir);
    connect(PushButtonRelocateDir,
            &QPushButton::clicked,
            this,
            &DlgPrefLibrary::slotRelocateDir);

    // Set default direction as stored in config file
    int rowHeight = m_pLibrary->getTrackTableRowHeight();
    spinBoxRowHeight->setValue(rowHeight);
    connect(spinBoxRowHeight, SIGNAL(valueChanged(int)),
            this, SLOT(slotRowHeightValueChanged(int)));

    searchDebouncingTimeoutSpinBox->setMinimum(WSearchLineEdit::kMinDebouncingTimeoutMillis);
    searchDebouncingTimeoutSpinBox->setMaximum(WSearchLineEdit::kMaxDebouncingTimeoutMillis);
    const auto searchDebouncingTimeoutMillis =
            m_pConfig->getValue(
                    ConfigKey("[Library]","SearchDebouncingTimeoutMillis"),
                    WSearchLineEdit::kDefaultDebouncingTimeoutMillis);
    searchDebouncingTimeoutSpinBox->setValue(searchDebouncingTimeoutMillis);
    connect(searchDebouncingTimeoutSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(slotSearchDebouncingTimeoutMillisChanged(int)));

    connect(libraryFontButton, SIGNAL(clicked()),
            this, SLOT(slotSelectFont()));

    // TODO(XXX) this string should be extracted from the soundsources
    QString builtInFormatsStr = "Ogg Vorbis, FLAC, WAVE, AIFF";
#if defined(__MAD__) || defined(__COREAUDIO__)
    builtInFormatsStr += ", MP3";
#endif
#if defined(__MEDIAFOUNDATION__) || defined(__COREAUDIO__) || defined(__FAAD__)
    builtInFormatsStr += ", M4A/MP4";
#endif
#ifdef __OPUS__
    builtInFormatsStr += ", Opus";
#endif
#ifdef __MODPLUG__
    builtInFormatsStr += ", ModPlug";
#endif
#ifdef __WV__
    builtInFormatsStr += ", WavPack";
#endif
    builtInFormats->setText(builtInFormatsStr);

    connect(checkBox_SyncTrackMetadataExport, SIGNAL(toggled(bool)),
            this, SLOT(slotSyncTrackMetadataExportToggled()));

    // Initialize the controls after all slots have been connected
    slotUpdate();
}

void DlgPrefLibrary::slotShow() {
    m_bAddedDirectory = false;
}

void DlgPrefLibrary::slotHide() {
    if (!m_bAddedDirectory) {
        return;
    }

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle(tr("Music Directory Added"));
    msgBox.setText(tr("You added one or more music directories. The tracks in "
                      "these directories won't be available until you rescan "
                      "your library. Would you like to rescan now?"));
    QPushButton* scanButton = msgBox.addButton(
        tr("Scan"), QMessageBox::AcceptRole);
    msgBox.addButton(QMessageBox::Cancel);
    msgBox.setDefaultButton(scanButton);
    msgBox.exec();

    if (msgBox.clickedButton() == scanButton) {
        emit scanLibrary();
        return;
    }
}

QUrl DlgPrefLibrary::helpUrl() const {
    return QUrl(MIXXX_MANUAL_LIBRARY_URL);
}

void DlgPrefLibrary::initializeDirList() {
    // save which index was selected
    const QString selected = dirList->currentIndex().data().toString();
    // clear and fill model
    m_dirListModel.clear();
    QStringList dirs = m_pLibrary->getDirs();
    foreach (QString dir, dirs) {
        m_dirListModel.appendRow(new QStandardItem(dir));
    }
    dirList->setModel(&m_dirListModel);
    dirList->setCurrentIndex(m_dirListModel.index(0, 0));
    // reselect index if it still exists
    for (int i=0 ; i<m_dirListModel.rowCount() ; ++i) {
        const QModelIndex index = m_dirListModel.index(i, 0);
        if (index.data().toString() == selected) {
            dirList->setCurrentIndex(index);
            break;
        }
    }
}

void DlgPrefLibrary::slotResetToDefaults() {
    checkBox_library_scan->setChecked(false);
    checkBox_SyncTrackMetadataExport->setChecked(false);
    checkBox_use_relative_path->setChecked(false);
    checkBox_show_rhythmbox->setChecked(true);
    checkBox_show_banshee->setChecked(true);
    checkBox_show_itunes->setChecked(true);
    checkBox_show_traktor->setChecked(true);
    checkBox_show_rekordbox->setChecked(true);
    radioButton_dbclick_bottom->setChecked(false);
    checkBoxEditMetadataSelectedClicked->setChecked(PREF_LIBRARY_EDIT_METADATA_DEFAULT);
    radioButton_dbclick_top->setChecked(false);
    radioButton_dbclick_deck->setChecked(true);
    spinBoxRowHeight->setValue(Library::kDefaultRowHeightPx);
    setLibraryFont(QApplication::font());
}

void DlgPrefLibrary::slotUpdate() {
    initializeDirList();
    checkBox_library_scan->setChecked(m_pConfig->getValue(
            ConfigKey("[Library]","RescanOnStartup"), false));
    checkBox_SyncTrackMetadataExport->setChecked(m_pConfig->getValue(
            ConfigKey("[Library]","SyncTrackMetadataExport"), false));
    checkBox_use_relative_path->setChecked(m_pConfig->getValue(
            ConfigKey("[Library]","UseRelativePathOnExport"), false));
    checkBox_show_rhythmbox->setChecked(m_pConfig->getValue(
            ConfigKey("[Library]","ShowRhythmboxLibrary"), true));
    checkBox_show_banshee->setChecked(m_pConfig->getValue(
            ConfigKey("[Library]","ShowBansheeLibrary"), true));
    checkBox_show_itunes->setChecked(m_pConfig->getValue(
            ConfigKey("[Library]","ShowITunesLibrary"), true));
    checkBox_show_traktor->setChecked(m_pConfig->getValue(
            ConfigKey("[Library]","ShowTraktorLibrary"), true));
    checkBox_show_rekordbox->setChecked(m_pConfig->getValue(
            ConfigKey("[Library]","ShowRekordboxLibrary"), true));
    checkBox_show_serato->setChecked(m_pConfig->getValue(
            ConfigKey("[Library]", "ShowSeratoLibrary"), true));

    switch (m_pConfig->getValue<int>(
            ConfigKey("[Library]", "TrackLoadAction"),
            static_cast<int>(TrackDoubleClickAction::LoadToDeck))) {
    case static_cast<int>(TrackDoubleClickAction::AddToAutoDJBottom):
        radioButton_dbclick_bottom->setChecked(true);
        break;
    case static_cast<int>(TrackDoubleClickAction::AddToAutoDJTop):
        radioButton_dbclick_top->setChecked(true);
        break;
    case static_cast<int>(TrackDoubleClickAction::Ignore):
        radioButton_dbclick_ignore->setChecked(true);
        break;
    default:
            radioButton_dbclick_deck->setChecked(true);
            break;
    }

    bool editMetadataSelectedClick = m_pConfig->getValue(
            ConfigKey("[Library]","EditMetadataSelectedClick"),
            PREF_LIBRARY_EDIT_METADATA_DEFAULT);
    checkBoxEditMetadataSelectedClicked->setChecked(editMetadataSelectedClick);
    m_pLibrary->setEditMedatataSelectedClick(editMetadataSelectedClick);

    m_originalTrackTableFont = m_pLibrary->getTrackTableFont();
    m_iOriginalTrackTableRowHeight = m_pLibrary->getTrackTableRowHeight();
    spinBoxRowHeight->setValue(m_iOriginalTrackTableRowHeight);
    setLibraryFont(m_originalTrackTableFont);
}

void DlgPrefLibrary::slotCancel() {
    // Undo any changes in the library font or row height.
    m_pLibrary->setFont(m_originalTrackTableFont);
    m_pLibrary->setRowHeight(m_iOriginalTrackTableRowHeight);
}

void DlgPrefLibrary::slotAddDir() {
    QString fd = QFileDialog::getExistingDirectory(
        this, tr("Choose a music directory"),
        QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
    if (!fd.isEmpty()) {
        emit requestAddDir(fd);
        slotUpdate();
        m_bAddedDirectory = true;
    }
}

void DlgPrefLibrary::slotRemoveDir() {
    QModelIndex index = dirList->currentIndex();
    QString fd = index.data().toString();
    QMessageBox removeMsgBox;

    removeMsgBox.setIcon(QMessageBox::Warning);
    removeMsgBox.setWindowTitle(tr("Confirm Directory Removal"));

    removeMsgBox.setText(tr(
        "Mixxx will no longer watch this directory for new tracks. "
        "What would you like to do with the tracks from this directory and "
        "subdirectories?"
        "<ul>"
        "<li>Hide all tracks from this directory and subdirectories.</li>"
        "<li>Delete all metadata for these tracks from Mixxx permanently.</li>"
        "<li>Leave the tracks unchanged in your library.</li>"
        "</ul>"
        "Hiding tracks saves their metadata in case you re-add them in the "
        "future."));
    removeMsgBox.setInformativeText(tr(
        "Metadata means all track details (artist, title, playcount, etc.) as "
        "well as beatgrids, hotcues, and loops. This choice only affects the "
        "Mixxx library. No files on disk will be changed or deleted."));

    QPushButton* cancelButton =
            removeMsgBox.addButton(QMessageBox::Cancel);
    QPushButton* hideAllButton = removeMsgBox.addButton(
        tr("Hide Tracks"), QMessageBox::AcceptRole);
    QPushButton* deleteAllButton = removeMsgBox.addButton(
        tr("Delete Track Metadata"), QMessageBox::AcceptRole);
    QPushButton* leaveUnchangedButton = removeMsgBox.addButton(
        tr("Leave Tracks Unchanged"), QMessageBox::AcceptRole);
    Q_UNUSED(leaveUnchangedButton); // Only used in DEBUG_ASSERT
    removeMsgBox.setDefaultButton(cancelButton);
    removeMsgBox.exec();

    if (removeMsgBox.clickedButton() == cancelButton) {
        return;
    }

    Library::RemovalType removalType;
    if (removeMsgBox.clickedButton() == hideAllButton) {
        removalType = Library::RemovalType::HideTracks;
    } else if (removeMsgBox.clickedButton() == deleteAllButton) {
        removalType = Library::RemovalType::PurgeTracks;
    } else {
        DEBUG_ASSERT(removeMsgBox.clickedButton() == leaveUnchangedButton);
        removalType = Library::RemovalType::KeepTracks;
    }

    emit requestRemoveDir(fd, removalType);
    slotUpdate();
}

void DlgPrefLibrary::slotRelocateDir() {
    QModelIndex index = dirList->currentIndex();
    QString currentFd = index.data().toString();

    // If the selected directory exists, use it. If not, go up one directory (if
    // that directory exists). If neither exist, use the default music
    // directory.
    QString startDir = currentFd;
    QDir dir(startDir);
    if (!dir.exists() && dir.cdUp()) {
        startDir = dir.absolutePath();
    } else if (!dir.exists()) {
        startDir = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    }

    QString fd = QFileDialog::getExistingDirectory(
        this, tr("Relink music directory to new location"), startDir);

    if (!fd.isEmpty()) {
        emit requestRelocateDir(currentFd, fd);
        slotUpdate();
    }
}

void DlgPrefLibrary::slotApply() {
    m_pConfig->set(ConfigKey("[Library]","RescanOnStartup"),
                ConfigValue((int)checkBox_library_scan->isChecked()));
    m_pConfig->set(ConfigKey("[Library]","SyncTrackMetadataExport"),
                ConfigValue((int)checkBox_SyncTrackMetadataExport->isChecked()));
    m_pConfig->set(ConfigKey("[Library]","UseRelativePathOnExport"),
                ConfigValue((int)checkBox_use_relative_path->isChecked()));
    m_pConfig->set(ConfigKey("[Library]","ShowRhythmboxLibrary"),
                ConfigValue((int)checkBox_show_rhythmbox->isChecked()));
    m_pConfig->set(ConfigKey("[Library]","ShowBansheeLibrary"),
                ConfigValue((int)checkBox_show_banshee->isChecked()));
    m_pConfig->set(ConfigKey("[Library]","ShowITunesLibrary"),
                ConfigValue((int)checkBox_show_itunes->isChecked()));
    m_pConfig->set(ConfigKey("[Library]","ShowTraktorLibrary"),
                ConfigValue((int)checkBox_show_traktor->isChecked()));
    m_pConfig->set(ConfigKey("[Library]","ShowRekordboxLibrary"),
                ConfigValue((int)checkBox_show_rekordbox->isChecked()));
    m_pConfig->set(ConfigKey("[Library]", "ShowSeratoLibrary"),
            ConfigValue((int)checkBox_show_serato->isChecked()));
    int dbclick_status;
    if (radioButton_dbclick_bottom->isChecked()) {
        dbclick_status = static_cast<int>(TrackDoubleClickAction::AddToAutoDJBottom);
    } else if (radioButton_dbclick_top->isChecked()) {
        dbclick_status = static_cast<int>(TrackDoubleClickAction::AddToAutoDJTop);
    } else if (radioButton_dbclick_deck->isChecked()) {
        dbclick_status = static_cast<int>(TrackDoubleClickAction::LoadToDeck);
    } else { // radioButton_dbclick_ignore
        dbclick_status = static_cast<int>(TrackDoubleClickAction::Ignore);
    }
    m_pConfig->set(ConfigKey("[Library]","TrackLoadAction"),
                ConfigValue(dbclick_status));

    m_pConfig->set(ConfigKey("[Library]", "EditMetadataSelectedClick"),
            ConfigValue(checkBoxEditMetadataSelectedClicked->checkState()));
    m_pLibrary->setEditMedatataSelectedClick(
            checkBoxEditMetadataSelectedClicked->checkState());

    QFont font = m_pLibrary->getTrackTableFont();
    if (m_originalTrackTableFont != font) {
        m_pConfig->set(ConfigKey("[Library]", "Font"),
                       ConfigValue(font.toString()));
    }

    int rowHeight = spinBoxRowHeight->value();
    if (m_iOriginalTrackTableRowHeight != rowHeight) {
        m_pConfig->set(ConfigKey("[Library]","RowHeight"),
                       ConfigValue(rowHeight));
    }

    // TODO(rryan): Don't save here.
    m_pConfig->save();
}

void DlgPrefLibrary::slotRowHeightValueChanged(int height) {
    m_pLibrary->setRowHeight(height);
}

void DlgPrefLibrary::setLibraryFont(const QFont& font) {
    libraryFont->setText(QString("%1 %2 %3pt").arg(
        font.family(), font.styleName(), QString::number(font.pointSizeF())));
    m_pLibrary->setFont(font);

    // Don't let the row height exceed the library height.
    QFontMetrics metrics(font);
    int fontHeight = metrics.height();
    if (fontHeight > spinBoxRowHeight->value()) {
        spinBoxRowHeight->setValue(fontHeight);
    }
    spinBoxRowHeight->setMinimum(fontHeight);
}

void DlgPrefLibrary::slotSelectFont() {
    // False if the user cancels font selection.
    bool ok = false;
    QFont font = QFontDialog::getFont(&ok, m_pLibrary->getTrackTableFont(),
                                      this, tr("Select Library Font"));
    if (ok) {
        setLibraryFont(font);
    }
}

void DlgPrefLibrary::slotSearchDebouncingTimeoutMillisChanged(int searchDebouncingTimeoutMillis) {
    m_pConfig->setValue(
            kSearchDebouncingTimeoutMillisKey,
            searchDebouncingTimeoutMillis);
    WSearchLineEdit::setDebouncingTimeoutMillis(searchDebouncingTimeoutMillis);
}

void DlgPrefLibrary::slotSyncTrackMetadataExportToggled() {
    if (isVisible() && checkBox_SyncTrackMetadataExport->isChecked()) {
        mixxx::DlgTrackMetadataExport::showMessageBoxOncePerSession();
    }
}
