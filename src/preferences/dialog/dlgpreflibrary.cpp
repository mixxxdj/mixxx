#include "preferences/dialog/dlgpreflibrary.h"

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFontDialog>
#include <QFontMetrics>
#include <QMessageBox>
#include <QStandardPaths>
#include <QStringList>
#include <QUrl>

#include "defs_urls.h"
#include "library/basetracktablemodel.h"
#include "library/dlgtrackmetadataexport.h"
#include "library/library.h"
#include "library/library_prefs.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "moc_dlgpreflibrary.cpp"
#include "sources/soundsourceproxy.h"
#include "widget/wsearchlineedit.h"

using namespace mixxx::library::prefs;

DlgPrefLibrary::DlgPrefLibrary(
        QWidget* pParent,
        UserSettingsPointer pConfig,
        std::shared_ptr<Library> pLibrary)
        : DlgPreferencePage(pParent),
          m_dirListModel(),
          m_pConfig(pConfig),
          m_pLibrary(pLibrary),
          m_bAddedDirectory(false),
          m_iOriginalTrackTableRowHeight(Library::kDefaultRowHeightPx) {
    setupUi(this);

    connect(this,
            &DlgPrefLibrary::requestAddDir,
            m_pLibrary.get(),
            &Library::slotRequestAddDir);
    connect(this,
            &DlgPrefLibrary::requestRemoveDir,
            m_pLibrary.get(),
            &Library::slotRequestRemoveDir);
    connect(this,
            &DlgPrefLibrary::requestRelocateDir,
            m_pLibrary.get(),
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
    connect(checkBox_SeratoMetadataExport,
            &QAbstractButton::clicked,
            this,
            &DlgPrefLibrary::slotSeratoMetadataExportClicked);
    const QString& settingsDir = m_pConfig->getSettingsPath();
    connect(PushButtonOpenSettingsDir,
            &QPushButton::clicked,
            [settingsDir] {
                QDesktopServices::openUrl(QUrl::fromLocalFile(settingsDir));
            });

    // Set default direction as stored in config file
    int rowHeight = m_pLibrary->getTrackTableRowHeight();
    spinBoxRowHeight->setValue(rowHeight);
    connect(spinBoxRowHeight,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefLibrary::slotRowHeightValueChanged);

    spinbox_bpm_precision->setMinimum(BaseTrackTableModel::kBpmColumnPrecisionMinimum);
    spinbox_bpm_precision->setMaximum(BaseTrackTableModel::kBpmColumnPrecisionMaximum);
    connect(spinbox_bpm_precision,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefLibrary::slotBpmColumnPrecisionChanged);

    searchDebouncingTimeoutSpinBox->setMinimum(WSearchLineEdit::kMinDebouncingTimeoutMillis);
    searchDebouncingTimeoutSpinBox->setMaximum(WSearchLineEdit::kMaxDebouncingTimeoutMillis);
    connect(searchDebouncingTimeoutSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefLibrary::slotSearchDebouncingTimeoutMillisChanged);

    updateSearchLineEditHistoryOptions();

    connect(libraryFontButton, &QAbstractButton::clicked, this, &DlgPrefLibrary::slotSelectFont);

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

    // Create text color manual links
    createLinkColor();
    // Add link to the manual where configuration files are explained in detail
    label_settingsManualLink->setText(coloredLinkString(
            m_pLinkColor,
            tr("See the manual for details"),
            MIXXX_MANUAL_SETTINGS_DIRECTORY_URL));
    connect(label_settingsManualLink,
            &QLabel::linkActivated,
            [](const QString& url) {
                QDesktopServices::openUrl(url);
            });

    connect(checkBox_SyncTrackMetadata,
            &QCheckBox::toggled,
            this,
            &DlgPrefLibrary::slotSyncTrackMetadataToggled);

    setScrollSafeGuardForAllInputWidgets(this);

    // Initialize the controls after all slots have been connected
    slotUpdate();
}

DlgPrefLibrary::~DlgPrefLibrary() = default;

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
    const auto rootDirs = m_pLibrary->trackCollectionManager()
                                  ->internalCollection()
                                  ->loadRootDirs();
    for (const mixxx::FileInfo& rootDir : rootDirs) {
        m_dirListModel.appendRow(new QStandardItem(rootDir.location()));
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
    spinbox_history_track_duplicate_distance->setValue(
            kHistoryTrackDuplicateDistanceDefault);
    spinbox_history_min_tracks_to_keep->setValue(1);
    checkBox_SyncTrackMetadata->setChecked(false);
    checkBox_SeratoMetadataExport->setChecked(false);
    checkBox_use_relative_path->setChecked(false);
    checkBoxEditMetadataSelectedClicked->setChecked(kEditMetadataSelectedClickDefault);
    radioButton_dbclick_deck->setChecked(true);
    spinbox_bpm_precision->setValue(BaseTrackTableModel::kBpmColumnPrecisionDefault);

    radioButton_cover_art_fetcher_medium->setChecked(true);

    spinBoxRowHeight->setValue(Library::kDefaultRowHeightPx);
    setLibraryFont(QApplication::font());
    searchDebouncingTimeoutSpinBox->setValue(
            WSearchLineEdit::kDefaultDebouncingTimeoutMillis);
    checkBoxEnableSearchCompletions->setChecked(WSearchLineEdit::kCompletionsEnabledDefault);
    checkBoxEnableSearchHistoryShortcuts->setChecked(
            WSearchLineEdit::kHistoryShortcutsEnabledDefault);

    checkBox_show_rhythmbox->setChecked(true);
    checkBox_show_banshee->setChecked(true);
    checkBox_show_itunes->setChecked(true);
    checkBox_show_traktor->setChecked(true);
    checkBox_show_rekordbox->setChecked(true);
}

void DlgPrefLibrary::slotUpdate() {
    initializeDirList();
    checkBox_library_scan->setChecked(m_pConfig->getValue(
            kRescanOnStartupConfigKey, false));

    spinbox_history_track_duplicate_distance->setValue(m_pConfig->getValue(
            kHistoryTrackDuplicateDistanceConfigKey,
            kHistoryTrackDuplicateDistanceDefault));
    spinbox_history_min_tracks_to_keep->setValue(m_pConfig->getValue(
            kHistoryMinTracksToKeepConfigKey,
            kHistoryMinTracksToKeepDefault));

    checkBox_SyncTrackMetadata->setChecked(
            m_pConfig->getValue(kSyncTrackMetadataConfigKey, false));
    checkBox_SeratoMetadataExport->setChecked(
            m_pConfig->getValue(kSyncSeratoMetadataConfigKey, false));
    setSeratoMetadataEnabled(checkBox_SyncTrackMetadata->isChecked());
    checkBox_use_relative_path->setChecked(m_pConfig->getValue(
            kUseRelativePathOnExportConfigKey, false));

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
            kTrackDoubleClickActionConfigKey,
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

    switch (m_pConfig->getValue<int>(
            kCoverArtFetcherQualityConfigKey,
            static_cast<int>(CoverArtFetcherQuality::Low))) {
    case static_cast<int>(CoverArtFetcherQuality::Highest):
        radioButton_cover_art_fetcher_highest->setChecked(true);
        break;
    case static_cast<int>(CoverArtFetcherQuality::High):
        radioButton_cover_art_fetcher_high->setChecked(true);
        break;
    case static_cast<int>(CoverArtFetcherQuality::Medium):
        radioButton_cover_art_fetcher_medium->setChecked(true);
        break;
    default:
        radioButton_cover_art_fetcher_lowest->setChecked(true);
        break;
    }

    bool editMetadataSelectedClick = m_pConfig->getValue(
            kEditMetadataSelectedClickConfigKey,
            kEditMetadataSelectedClickDefault);
    checkBoxEditMetadataSelectedClicked->setChecked(editMetadataSelectedClick);
    m_pLibrary->setEditMedatataSelectedClick(editMetadataSelectedClick);

    checkBoxEnableSearchCompletions->setChecked(m_pConfig->getValue(
            kEnableSearchCompletionsConfigKey,
            WSearchLineEdit::kCompletionsEnabledDefault));
    checkBoxEnableSearchHistoryShortcuts->setChecked(m_pConfig->getValue(
            kEnableSearchHistoryShortcutsConfigKey,
            WSearchLineEdit::kHistoryShortcutsEnabledDefault));

    m_originalTrackTableFont = m_pLibrary->getTrackTableFont();
    m_iOriginalTrackTableRowHeight = m_pLibrary->getTrackTableRowHeight();
    spinBoxRowHeight->setValue(m_iOriginalTrackTableRowHeight);
    setLibraryFont(m_originalTrackTableFont);

    const auto searchDebouncingTimeoutMillis =
            m_pConfig->getValue(
                    kSearchDebouncingTimeoutMillisConfigKey,
                    WSearchLineEdit::kDefaultDebouncingTimeoutMillis);
    searchDebouncingTimeoutSpinBox->setValue(searchDebouncingTimeoutMillis);

    const auto bpmColumnPrecision =
            m_pConfig->getValue(
                    kBpmColumnPrecisionConfigKey,
                    BaseTrackTableModel::kBpmColumnPrecisionDefault);
    spinbox_bpm_precision->setValue(bpmColumnPrecision);
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

    LibraryRemovalType removalType;
    if (removeMsgBox.clickedButton() == hideAllButton) {
        removalType = LibraryRemovalType::HideTracks;
    } else if (removeMsgBox.clickedButton() == deleteAllButton) {
        removalType = LibraryRemovalType::PurgeTracks;
    } else {
        // Only used in DEBUG_ASSERT
        Q_UNUSED(leaveUnchangedButton);
        DEBUG_ASSERT(removeMsgBox.clickedButton() == leaveUnchangedButton);
        removalType = LibraryRemovalType::KeepTracks;
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

void DlgPrefLibrary::slotSeratoMetadataExportClicked(bool checked) {
    if (checked) {
        if (QMessageBox::warning(this,
                    QStringLiteral("Serato Metadata Export"),
                    QStringLiteral(
                            "Exporting Serato Metadata from Mixxx is "
                            "experimental. There is no official documentation "
                            "of the format. Existing Serato Metadata might be "
                            "lost and files with Serato metadata written by "
                            "Mixxx could potentially crash Serato DJ, "
                            "therefore caution is advised and backups are "
                            "recommended. Are you sure you want to enable this "
                            "option?"),
                    QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
            checkBox_SeratoMetadataExport->setChecked(false);
        }
    }
}

void DlgPrefLibrary::slotApply() {
    m_pConfig->set(kRescanOnStartupConfigKey,
            ConfigValue((int)checkBox_library_scan->isChecked()));

    m_pConfig->set(kHistoryTrackDuplicateDistanceConfigKey,
            ConfigValue(spinbox_history_track_duplicate_distance->value()));
    m_pConfig->set(kHistoryMinTracksToKeepConfigKey,
            ConfigValue(spinbox_history_min_tracks_to_keep->value()));

    m_pConfig->set(
            kSyncTrackMetadataConfigKey,
            ConfigValue{checkBox_SyncTrackMetadata->isChecked()});
    m_pConfig->set(
            kSyncSeratoMetadataConfigKey,
            ConfigValue{checkBox_SeratoMetadataExport->isChecked()});

    m_pConfig->set(kUseRelativePathOnExportConfigKey,
            ConfigValue((int)checkBox_use_relative_path->isChecked()));

    m_pConfig->set(kEnableSearchCompletionsConfigKey,
            ConfigValue(checkBoxEnableSearchCompletions->isChecked()));
    m_pConfig->set(kEnableSearchHistoryShortcutsConfigKey,
            ConfigValue(checkBoxEnableSearchHistoryShortcuts->isChecked()));
    updateSearchLineEditHistoryOptions();

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

    int coverartfetcherquality_status;
    if (radioButton_cover_art_fetcher_highest->isChecked()) {
        coverartfetcherquality_status = static_cast<int>(CoverArtFetcherQuality::Highest);
    } else if (radioButton_cover_art_fetcher_high->isChecked()) {
        coverartfetcherquality_status = static_cast<int>(CoverArtFetcherQuality::High);
    } else if (radioButton_cover_art_fetcher_medium->isChecked()) {
        coverartfetcherquality_status = static_cast<int>(CoverArtFetcherQuality::Medium);
    } else {
        coverartfetcherquality_status = static_cast<int>(CoverArtFetcherQuality::Low);
    }
    m_pConfig->set(kCoverArtFetcherQualityConfigKey, ConfigValue(coverartfetcherquality_status));

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
    m_pConfig->set(kTrackDoubleClickActionConfigKey,
            ConfigValue(dbclick_status));

    m_pConfig->set(kEditMetadataSelectedClickConfigKey,
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

    // Don't let the font height exceed the row height.
    QFontMetrics metrics(font);
    int fontHeight = metrics.height();
    spinBoxRowHeight->setMinimum(fontHeight);
    // library.cpp takes care of setting the new row height according to the
    // previous font height/ row height ratio
    spinBoxRowHeight->setValue(m_pLibrary->getTrackTableRowHeight());
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
            kSearchDebouncingTimeoutMillisConfigKey,
            searchDebouncingTimeoutMillis);
    WSearchLineEdit::setDebouncingTimeoutMillis(searchDebouncingTimeoutMillis);
}

void DlgPrefLibrary::updateSearchLineEditHistoryOptions() {
    WSearchLineEdit::setSearchCompletionsEnabled(m_pConfig->getValue<bool>(
            kEnableSearchCompletionsConfigKey,
            WSearchLineEdit::kCompletionsEnabledDefault));
    WSearchLineEdit::setSearchHistoryShortcutsEnabled(m_pConfig->getValue<bool>(
            kEnableSearchHistoryShortcutsConfigKey,
            WSearchLineEdit::kHistoryShortcutsEnabledDefault));
}

void DlgPrefLibrary::slotBpmColumnPrecisionChanged(int bpmPrecision) {
    m_pConfig->setValue(
            kBpmColumnPrecisionConfigKey,
            bpmPrecision);
    BaseTrackTableModel::setBpmColumnPrecision(bpmPrecision);
}

void DlgPrefLibrary::slotSyncTrackMetadataToggled() {
    bool shouldSyncTrackMetadata = checkBox_SyncTrackMetadata->isChecked();
    if (isVisible() && shouldSyncTrackMetadata) {
        mixxx::DlgTrackMetadataExport::showMessageBoxOncePerSession();
    }
    setSeratoMetadataEnabled(shouldSyncTrackMetadata);
}

void DlgPrefLibrary::setSeratoMetadataEnabled(bool shouldSyncTrackMetadata) {
    checkBox_SeratoMetadataExport->setEnabled(shouldSyncTrackMetadata);
    if (!shouldSyncTrackMetadata) {
        checkBox_SeratoMetadataExport->setChecked(false);
    }
}
