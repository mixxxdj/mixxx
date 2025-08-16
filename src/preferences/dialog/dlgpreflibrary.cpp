#include "preferences/dialog/dlgpreflibrary.h"

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QFontDialog>
#include <QFontMetrics>
#include <QMessageBox>
#include <QStandardPaths>
#include <QUrl>
#include <QtGlobal>

#include "control/controlproxy.h"
#include "defs_urls.h"
#include "library/basetracktablemodel.h"
#include "library/dlgtrackmetadataexport.h"
#include "library/library.h"
#include "library/library_prefs.h"
#include "library/searchquery.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "moc_dlgpreflibrary.cpp"
#include "util/desktophelper.h"
#include "widget/wsearchlineedit.h"

namespace {
constexpr int kDefaultFuzzyRateRangePercent = 75;
} // namespace

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
          m_iOriginalTrackTableRowHeight(Library::kDefaultRowHeightPx),
          m_pRateRangeDeck1(make_parented<ControlProxy>(
                  QStringLiteral("[Channel1]"), QStringLiteral("rateRange"), this)) {
    setupUi(this);

    connect(pushButton_add_dir,
            &QPushButton::clicked,
            this,
            &DlgPrefLibrary::slotAddDir);
    connect(pushButton_remove_dir,
            &QPushButton::clicked,
            this,
            &DlgPrefLibrary::slotRemoveDir);
    connect(pushButton_relocate_dir,
            &QPushButton::clicked,
            this,
            &DlgPrefLibrary::slotRelocateDir);
    connect(checkBox_serato_metadata_export,
            &QAbstractButton::clicked,
            this,
            &DlgPrefLibrary::slotSeratoMetadataExportClicked);
    const QString& settingsDir = m_pConfig->getSettingsPath();
    connect(pushButton_open_settings_dir,
            &QPushButton::clicked,
            [settingsDir] {
                mixxx::DesktopHelper::openUrl(QUrl::fromLocalFile(settingsDir));
            });

    // Set default direction as stored in config file
    int rowHeight = m_pLibrary->getTrackTableRowHeight();
    spinBox_row_height->setValue(rowHeight);
    connect(spinBox_row_height,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefLibrary::slotRowHeightValueChanged);

    spinbox_bpm_precision->setMinimum(BaseTrackTableModel::kBpmColumnPrecisionMinimum);
    spinbox_bpm_precision->setMaximum(BaseTrackTableModel::kBpmColumnPrecisionMaximum);
    connect(spinbox_bpm_precision,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefLibrary::slotBpmColumnPrecisionChanged);

    spinBox_search_debouncing_timeout->setMinimum(WSearchLineEdit::kMinDebouncingTimeoutMillis);
    spinBox_search_debouncing_timeout->setMaximum(WSearchLineEdit::kMaxDebouncingTimeoutMillis);
    connect(spinBox_search_debouncing_timeout,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefLibrary::slotSearchDebouncingTimeoutMillisChanged);

#ifdef Q_OS_IOS
    checkBox_edit_metadata_selected_clicked->setEnabled(false);
#endif

    comboBox_search_bpm_fuzzy_range->clear();
    comboBox_search_bpm_fuzzy_range->addItem("25 %", 25);
    comboBox_search_bpm_fuzzy_range->addItem("50 %", 50);
    comboBox_search_bpm_fuzzy_range->addItem("75 %", 75);
    comboBox_search_bpm_fuzzy_range->addItem("100 %", 100);
    connect(comboBox_search_bpm_fuzzy_range,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefLibrary::slotBpmRangeSelected);
    // Also listen to rate range changes (e.g. made in DlgPrefDecks) and
    // adjust the fuzzy range accordingly
    m_pRateRangeDeck1->connectValueChanged(
            this,
            [this]() {
                slotBpmRangeSelected(comboBox_search_bpm_fuzzy_range->currentIndex());
            });

    updateSearchLineEditHistoryOptions();

    connect(btn_library_font, &QAbstractButton::clicked, this, &DlgPrefLibrary::slotSelectFont);

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
    // TODO It seems this isnot required anymore with Qt 6.2.3
    connect(label_settingsManualLink,
            &QLabel::linkActivated,
            [](const QString& url) {
                mixxx::DesktopHelper::openUrl(url);
            });

    // Add link to the track search documentation
    label_searchBpmFuzzyRangeInfo->setText(
            label_searchBpmFuzzyRangeInfo->text() + QStringLiteral(" ") +
            coloredLinkString(m_pLinkColor,
                    QStringLiteral("(?)"),
                    MIXXX_MANUAL_SETTINGS_DIRECTORY_URL));
    connect(label_searchBpmFuzzyRangeInfo,
            &QLabel::linkActivated,
            [](const QString& url) {
                mixxx::DesktopHelper::openUrl(url);
            });

    connect(checkBox_sync_track_metadata,
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

void DlgPrefLibrary::populateDirList() {
    // save which index was selected
    const QString selected = dirList->currentIndex().data().toString();
    // clear and fill model
    m_dirListModel.clear();
    const auto rootDirs = m_pLibrary->trackCollectionManager()
                                  ->internalCollection()
                                  ->getRootDirStrings();
    for (const QString& rootDir : rootDirs) {
        auto* pDirItem = new QStandardItem(rootDir);
        // Note: constructing a FileInfo from a path string added in another
        // will create issues: on Windows, if that path doesn't start with
        // '[drive letter]:' it'll get prefixed with 'C:'; if on Linux the path
        // starts with '[drive letter]:' the working dir's path is prepended.
        // In both cases this is obviously wrong and directory/track relocation
        // will fail since the database has no tracks with constructed prefix.
        // Let's use QStrings for the roundtrip. The FileInfo is just for
        // validation and eventually adding the warning icon.
        const mixxx::FileInfo fileInfo(rootDir);
        if (!fileInfo.exists() || !fileInfo.isDir()) {
            pDirItem->setIcon(QIcon(kWarningIconPath));
            pDirItem->setToolTip(tr("Item is not a directory or directory is missing"));
        }
        m_dirListModel.appendRow(pDirItem);
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
    checkBox_sync_track_metadata->setChecked(false);
    checkBox_serato_metadata_export->setChecked(false);
    checkBox_use_relative_path->setChecked(false);
    checkBox_edit_metadata_selected_clicked->setChecked(kEditMetadataSelectedClickDefault);
    radioButton_dbclick_deck->setChecked(true);
    spinbox_bpm_precision->setValue(BaseTrackTableModel::kBpmColumnPrecisionDefault);
    checkbox_played_track_color->setChecked(
            BaseTrackTableModel::kApplyPlayedTrackColorDefault);

    radioButton_cover_art_fetcher_medium->setChecked(true);

    spinBox_row_height->setValue(Library::kDefaultRowHeightPx);
    setLibraryFont(QApplication::font());
    spinBox_search_debouncing_timeout->setValue(
            WSearchLineEdit::kDefaultDebouncingTimeoutMillis);
    checkBox_enable_search_completions->setChecked(
            WSearchLineEdit::kCompletionsEnabledDefault);
    checkBox_enable_search_history_shortcuts->setChecked(
            WSearchLineEdit::kHistoryShortcutsEnabledDefault);
    comboBox_search_bpm_fuzzy_range->setCurrentIndex(
            comboBox_search_bpm_fuzzy_range->findData(kDefaultFuzzyRateRangePercent));

    checkBox_show_rhythmbox->setChecked(true);
    checkBox_show_banshee->setChecked(true);
    checkBox_show_itunes->setChecked(true);
    checkBox_show_traktor->setChecked(true);
    checkBox_show_rekordbox->setChecked(true);
}

void DlgPrefLibrary::slotUpdate() {
    populateDirList();
    checkBox_library_scan->setChecked(m_pConfig->getValue(
            kRescanOnStartupConfigKey, false));
    checkBox_library_scan_summary->setChecked(m_pConfig->getValue(
            kShowScanSummaryConfigKey, true));

    spinbox_history_track_duplicate_distance->setValue(m_pConfig->getValue(
            kHistoryTrackDuplicateDistanceConfigKey,
            kHistoryTrackDuplicateDistanceDefault));
    spinbox_history_min_tracks_to_keep->setValue(m_pConfig->getValue(
            kHistoryMinTracksToKeepConfigKey,
            kHistoryMinTracksToKeepDefault));

    checkBox_sync_track_metadata->setChecked(
            m_pConfig->getValue(kSyncTrackMetadataConfigKey, false));
    checkBox_serato_metadata_export->setChecked(
            m_pConfig->getValue(kSyncSeratoMetadataConfigKey, false));
    setSeratoMetadataEnabled(checkBox_sync_track_metadata->isChecked());
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
    checkBox_edit_metadata_selected_clicked->setChecked(editMetadataSelectedClick);
    m_pLibrary->setEditMetadataSelectedClick(editMetadataSelectedClick);

    checkBox_enable_search_completions->setChecked(m_pConfig->getValue(
            kEnableSearchCompletionsConfigKey,
            WSearchLineEdit::kCompletionsEnabledDefault));
    checkBox_enable_search_history_shortcuts->setChecked(m_pConfig->getValue(
            kEnableSearchHistoryShortcutsConfigKey,
            WSearchLineEdit::kHistoryShortcutsEnabledDefault));

    m_originalTrackTableFont = m_pLibrary->getTrackTableFont();
    m_iOriginalTrackTableRowHeight = m_pLibrary->getTrackTableRowHeight();
    spinBox_row_height->setValue(m_iOriginalTrackTableRowHeight);
    setLibraryFont(m_originalTrackTableFont);

    const auto searchDebouncingTimeoutMillis =
            m_pConfig->getValue(
                    kSearchDebouncingTimeoutMillisConfigKey,
                    WSearchLineEdit::kDefaultDebouncingTimeoutMillis);
    spinBox_search_debouncing_timeout->setValue(searchDebouncingTimeoutMillis);

    const auto searchBpmFuzzyRange =
            m_pConfig->getValue(
                    kSearchBpmFuzzyRangeConfigKey,
                    BpmFilterNode::kRelativeRangeDefault);
    int index = comboBox_search_bpm_fuzzy_range->findData(static_cast<int>(searchBpmFuzzyRange));
    if (index == -1) {
        index = comboBox_search_bpm_fuzzy_range->findData(kDefaultFuzzyRateRangePercent);
    }
    comboBox_search_bpm_fuzzy_range->setCurrentIndex(index);
    slotBpmRangeSelected(index);

    const auto bpmColumnPrecision =
            m_pConfig->getValue(
                    kBpmColumnPrecisionConfigKey,
                    BaseTrackTableModel::kBpmColumnPrecisionDefault);
    spinbox_bpm_precision->setValue(bpmColumnPrecision);

    const auto applyPlayedTrackColor =
            m_pConfig->getValue(
                    mixxx::library::prefs::kApplyPlayedTrackColorConfigKey,
                    BaseTrackTableModel::kApplyPlayedTrackColorDefault);
    checkbox_played_track_color->setChecked(applyPlayedTrackColor);
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
        if (m_pLibrary->requestAddDir(fd)) {
            populateDirList();
            m_bAddedDirectory = true;
        }
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

    if (m_pLibrary->requestRemoveDir(fd, removalType)) {
        populateDirList();
    }
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

    if (!fd.isEmpty() && m_pLibrary->requestRelocateDir(currentFd, fd)) {
        populateDirList();
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
            checkBox_serato_metadata_export->setChecked(false);
        }
    }
}

void DlgPrefLibrary::slotApply() {
    m_pConfig->set(kRescanOnStartupConfigKey,
            ConfigValue((int)checkBox_library_scan->isChecked()));

    m_pConfig->set(kShowScanSummaryConfigKey,
            ConfigValue((int)checkBox_library_scan_summary->isChecked()));

    m_pConfig->set(kHistoryTrackDuplicateDistanceConfigKey,
            ConfigValue(spinbox_history_track_duplicate_distance->value()));
    m_pConfig->set(kHistoryMinTracksToKeepConfigKey,
            ConfigValue(spinbox_history_min_tracks_to_keep->value()));

    m_pConfig->set(
            kSyncTrackMetadataConfigKey,
            ConfigValue{checkBox_sync_track_metadata->isChecked()});
    m_pConfig->set(
            kSyncSeratoMetadataConfigKey,
            ConfigValue{checkBox_serato_metadata_export->isChecked()});

    m_pConfig->set(kUseRelativePathOnExportConfigKey,
            ConfigValue((int)checkBox_use_relative_path->isChecked()));

    m_pConfig->set(kEnableSearchCompletionsConfigKey,
            ConfigValue(checkBox_enable_search_completions->isChecked()));
    m_pConfig->set(kEnableSearchHistoryShortcutsConfigKey,
            ConfigValue(checkBox_enable_search_history_shortcuts->isChecked()));
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
            ConfigValue(checkBox_edit_metadata_selected_clicked->checkState()));
    m_pLibrary->setEditMetadataSelectedClick(
            checkBox_edit_metadata_selected_clicked->checkState());

    QFont font = m_pLibrary->getTrackTableFont();
    if (m_originalTrackTableFont != font) {
        m_pConfig->set(ConfigKey("[Library]", "Font"),
                       ConfigValue(font.toString()));
    }

    int rowHeight = spinBox_row_height->value();
    if (m_iOriginalTrackTableRowHeight != rowHeight) {
        m_pConfig->set(ConfigKey("[Library]","RowHeight"),
                       ConfigValue(rowHeight));
    }

    BaseTrackTableModel::setApplyPlayedTrackColor(
            checkbox_played_track_color->isChecked());
    m_pConfig->set(
            mixxx::library::prefs::kApplyPlayedTrackColorConfigKey,
            ConfigValue(checkbox_played_track_color->isChecked()));

    // TODO(rryan): Don't save here.
    m_pConfig->save();
}

void DlgPrefLibrary::slotRowHeightValueChanged(int height) {
    m_pLibrary->setRowHeight(height);
}

void DlgPrefLibrary::setLibraryFont(const QFont& font) {
    lineEdit_library_font->setText(
            QString("%1 %2 %3pt")
                    .arg(font.family(),
                            font.styleName(),
                            QString::number(font.pointSizeF())));
    m_pLibrary->setFont(font);

    // Don't let the font height exceed the row height.
    QFontMetrics metrics(font);
    int fontHeight = metrics.height();
    spinBox_row_height->setMinimum(fontHeight);
    // library.cpp takes care of setting the new row height according to the
    // previous font height/ row height ratio
    spinBox_row_height->setValue(m_pLibrary->getTrackTableRowHeight());
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

void DlgPrefLibrary::slotBpmRangeSelected(int index) {
    const int bpmRange = comboBox_search_bpm_fuzzy_range->itemData(index).toInt();
    m_pConfig->set(kSearchBpmFuzzyRangeConfigKey, ConfigValue{bpmRange});
    const int rateRangePercent =
            m_pConfig->getValue(ConfigKey("[Controls]", "RateRangePercent"), 8);
    BpmFilterNode::setBpmRelativeRange(bpmRange * rateRangePercent / 10000.0);
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
    bool shouldSyncTrackMetadata = checkBox_sync_track_metadata->isChecked();
    if (isVisible() && shouldSyncTrackMetadata) {
        mixxx::DlgTrackMetadataExport::showMessageBoxOncePerSession();
    }
    setSeratoMetadataEnabled(shouldSyncTrackMetadata);
}

void DlgPrefLibrary::setSeratoMetadataEnabled(bool shouldSyncTrackMetadata) {
    checkBox_serato_metadata_export->setEnabled(shouldSyncTrackMetadata);
    if (!shouldSyncTrackMetadata) {
        checkBox_serato_metadata_export->setChecked(false);
    }
}
