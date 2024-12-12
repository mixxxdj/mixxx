/********************************************************************************
** Form generated from reading UI file 'dlgpreflibrarydlg.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGPREFLIBRARYDLG_H
#define UI_DLGPREFLIBRARYDLG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DlgPrefLibraryDlg {
  public:
    QVBoxLayout* verticalLayout;
    QGroupBox* groupBox_MusicDirectories;
    QGridLayout* gridLayout_music_directories;
    QListView* dirList;
    QPushButton* pushButton_add_dir;
    QPushButton* pushButton_relocate_dir;
    QPushButton* pushButton_remove_dir;
    QCheckBox* checkBox_library_scan;
    QGroupBox* groupBox_AudioFileFormats;
    QGridLayout* gridLayout_audio_formats;
    QLabel* builtInFormats;
    QGroupBox* groupBox_AudioFileTags;
    QGridLayout* gridLayout_metadata;
    QCheckBox* checkBox_sync_track_metadata;
    QCheckBox* checkBox_serato_metadata_export;
    QCheckBox* checkBox_use_relative_path;
    QGroupBox* groupBox_TrackTableView;
    QGridLayout* gridLayout_track_table_view;
    QCheckBox* checkBox_edit_metadata_selected_clicked;
    QLabel* label_doubeClickAction;
    QRadioButton* radioButton_dbclick_deck;
    QRadioButton* radioButton_dbclick_bottom;
    QRadioButton* radioButton_dbclick_top;
    QRadioButton* radioButton_dbclick_ignore;
    QLabel* label_rowHeight;
    QSpinBox* spinBox_row_height;
    QLabel* label_libraryFont;
    QLineEdit* lineEdit_library_font;
    QToolButton* btn_library_font;
    QLabel* label_bpm_precision;
    QSpinBox* spinbox_bpm_precision;
    QCheckBox* checkbox_played_track_color;
    QGroupBox* groupBox_Search;
    QGridLayout* gridLayout_search;
    QLabel* label_searchDebouncingTimeout;
    QSpinBox* spinBox_search_debouncing_timeout;
    QCheckBox* checkBox_enable_search_completions;
    QCheckBox* checkBox_enable_search_history_shortcuts;
    QLabel* label_searchBpmFuzzyRange;
    QComboBox* comboBox_search_bpm_fuzzy_range;
    QLabel* label_searchBpmFuzzyRangeInfo;
    QGroupBox* groupBox_History;
    QGridLayout* gridLayout_history;
    QLabel* label_history_track_duplicate_distance;
    QSpinBox* spinbox_history_track_duplicate_distance;
    QLabel* label_history_cleanup;
    QSpinBox* spinbox_history_min_tracks_to_keep;
    QGroupBox* groupBox_GroupedCrates;
    QCheckBox* checkBox_grouped_crates_enable;
    QRadioButton* radioButton_grouped_crates_fixed_length;
    QRadioButton* radioButton_grouped_crates_var_mask;
    QSpinBox* spinBox_grouped_crates_fixed_length;
    QLineEdit* lineEdit_grouped_crates_var_mask;
    QGroupBox* groupBox_external_libraries;
    QGridLayout* gridLayout_external_libraries;
    QCheckBox* checkBox_show_rhythmbox;
    QCheckBox* checkBox_show_banshee;
    QCheckBox* checkBox_show_itunes;
    QCheckBox* checkBox_show_traktor;
    QCheckBox* checkBox_show_rekordbox;
    QCheckBox* checkBox_show_serato;
    QLabel* label_4;
    QLabel* label_11;
    QGroupBox* groupBox_cover_art_fetcher;
    QGridLayout* gridLayout_cover_art_fetcher;
    QLabel* label_12;
    QLabel* label_13;
    QRadioButton* radioButton_cover_art_fetcher_highest;
    QRadioButton* radioButton_cover_art_fetcher_high;
    QRadioButton* radioButton_cover_art_fetcher_medium;
    QRadioButton* radioButton_cover_art_fetcher_lowest;
    QGroupBox* groupBox_settingsDir;
    QVBoxLayout* vlayout_settingsDir;
    QLabel* label_settingsDir;
    QLabel* label_settingsManualLink;
    QLabel* label_settingsWarning;
    QPushButton* pushButton_open_settings_dir;
    QSpacerItem* verticalSpacer;

    void setupUi(QWidget* DlgPrefLibraryDlg) {
        if (DlgPrefLibraryDlg->objectName().isEmpty())
            DlgPrefLibraryDlg->setObjectName("DlgPrefLibraryDlg");
        DlgPrefLibraryDlg->resize(661, 1522);
        DlgPrefLibraryDlg->setWindowTitle(QString::fromUtf8("Library Preferences"));
        verticalLayout = new QVBoxLayout(DlgPrefLibraryDlg);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName("verticalLayout");
        groupBox_MusicDirectories = new QGroupBox(DlgPrefLibraryDlg);
        groupBox_MusicDirectories->setObjectName("groupBox_MusicDirectories");
        gridLayout_music_directories = new QGridLayout(groupBox_MusicDirectories);
        gridLayout_music_directories->setSpacing(6);
        gridLayout_music_directories->setContentsMargins(11, 11, 11, 11);
        gridLayout_music_directories->setObjectName("gridLayout_music_directories");
        dirList = new QListView(groupBox_MusicDirectories);
        dirList->setObjectName("dirList");
        dirList->setEditTriggers(QAbstractItemView::NoEditTriggers);
        dirList->setIconSize(QSize(16, 16));
        dirList->setUniformItemSizes(true);

        gridLayout_music_directories->addWidget(dirList, 0, 0, 3, 1);

        pushButton_add_dir = new QPushButton(groupBox_MusicDirectories);
        pushButton_add_dir->setObjectName("pushButton_add_dir");
        QFont font;
        pushButton_add_dir->setFont(font);

        gridLayout_music_directories->addWidget(pushButton_add_dir, 0, 1, 1, 1);

        pushButton_relocate_dir = new QPushButton(groupBox_MusicDirectories);
        pushButton_relocate_dir->setObjectName("pushButton_relocate_dir");

        gridLayout_music_directories->addWidget(pushButton_relocate_dir, 1, 1, 1, 1);

        pushButton_remove_dir = new QPushButton(groupBox_MusicDirectories);
        pushButton_remove_dir->setObjectName("pushButton_remove_dir");
        pushButton_remove_dir->setFont(font);

        gridLayout_music_directories->addWidget(pushButton_remove_dir, 2, 1, 1, 1);

        checkBox_library_scan = new QCheckBox(groupBox_MusicDirectories);
        checkBox_library_scan->setObjectName("checkBox_library_scan");
        checkBox_library_scan->setChecked(true);

        gridLayout_music_directories->addWidget(checkBox_library_scan, 3, 0, 1, 2);

        verticalLayout->addWidget(groupBox_MusicDirectories);

        groupBox_AudioFileFormats = new QGroupBox(DlgPrefLibraryDlg);
        groupBox_AudioFileFormats->setObjectName("groupBox_AudioFileFormats");
        groupBox_AudioFileFormats->setFlat(false);
        gridLayout_audio_formats = new QGridLayout(groupBox_AudioFileFormats);
        gridLayout_audio_formats->setSpacing(6);
        gridLayout_audio_formats->setContentsMargins(11, 11, 11, 11);
        gridLayout_audio_formats->setObjectName("gridLayout_audio_formats");
        builtInFormats = new QLabel(groupBox_AudioFileFormats);
        builtInFormats->setObjectName("builtInFormats");
        builtInFormats->setWordWrap(true);

        gridLayout_audio_formats->addWidget(builtInFormats, 0, 0, 1, 1);

        verticalLayout->addWidget(groupBox_AudioFileFormats);

        groupBox_AudioFileTags = new QGroupBox(DlgPrefLibraryDlg);
        groupBox_AudioFileTags->setObjectName("groupBox_AudioFileTags");
        gridLayout_metadata = new QGridLayout(groupBox_AudioFileTags);
        gridLayout_metadata->setSpacing(6);
        gridLayout_metadata->setContentsMargins(11, 11, 11, 11);
        gridLayout_metadata->setObjectName("gridLayout_metadata");
        checkBox_sync_track_metadata = new QCheckBox(groupBox_AudioFileTags);
        checkBox_sync_track_metadata->setObjectName("checkBox_sync_track_metadata");

        gridLayout_metadata->addWidget(checkBox_sync_track_metadata, 0, 0, 1, 2);

        checkBox_serato_metadata_export = new QCheckBox(groupBox_AudioFileTags);
        checkBox_serato_metadata_export->setObjectName("checkBox_serato_metadata_export");

        gridLayout_metadata->addWidget(checkBox_serato_metadata_export, 1, 0, 1, 1);

        checkBox_use_relative_path = new QCheckBox(groupBox_AudioFileTags);
        checkBox_use_relative_path->setObjectName("checkBox_use_relative_path");

        gridLayout_metadata->addWidget(checkBox_use_relative_path, 2, 0, 1, 2);

        verticalLayout->addWidget(groupBox_AudioFileTags);

        groupBox_TrackTableView = new QGroupBox(DlgPrefLibraryDlg);
        groupBox_TrackTableView->setObjectName("groupBox_TrackTableView");
        gridLayout_track_table_view = new QGridLayout(groupBox_TrackTableView);
        gridLayout_track_table_view->setSpacing(6);
        gridLayout_track_table_view->setContentsMargins(11, 11, 11, 11);
        gridLayout_track_table_view->setObjectName("gridLayout_track_table_view");
        checkBox_edit_metadata_selected_clicked = new QCheckBox(groupBox_TrackTableView);
        checkBox_edit_metadata_selected_clicked->setObjectName(
                "checkBox_edit_metadata_selected_clicked");

        gridLayout_track_table_view->addWidget(checkBox_edit_metadata_selected_clicked, 1, 0, 1, 3);

        label_doubeClickAction = new QLabel(groupBox_TrackTableView);
        label_doubeClickAction->setObjectName("label_doubeClickAction");
        label_doubeClickAction->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignVCenter);

        gridLayout_track_table_view->addWidget(label_doubeClickAction, 2, 0, 1, 3);

        radioButton_dbclick_deck = new QRadioButton(groupBox_TrackTableView);
        radioButton_dbclick_deck->setObjectName("radioButton_dbclick_deck");
        radioButton_dbclick_deck->setChecked(true);

        gridLayout_track_table_view->addWidget(radioButton_dbclick_deck, 3, 0, 1, 3);

        radioButton_dbclick_bottom = new QRadioButton(groupBox_TrackTableView);
        radioButton_dbclick_bottom->setObjectName("radioButton_dbclick_bottom");

        gridLayout_track_table_view->addWidget(radioButton_dbclick_bottom, 4, 0, 1, 3);

        radioButton_dbclick_top = new QRadioButton(groupBox_TrackTableView);
        radioButton_dbclick_top->setObjectName("radioButton_dbclick_top");

        gridLayout_track_table_view->addWidget(radioButton_dbclick_top, 5, 0, 1, 3);

        radioButton_dbclick_ignore = new QRadioButton(groupBox_TrackTableView);
        radioButton_dbclick_ignore->setObjectName("radioButton_dbclick_ignore");

        gridLayout_track_table_view->addWidget(radioButton_dbclick_ignore, 6, 0, 1, 3);

        label_rowHeight = new QLabel(groupBox_TrackTableView);
        label_rowHeight->setObjectName("label_rowHeight");
        label_rowHeight->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignVCenter);

        gridLayout_track_table_view->addWidget(label_rowHeight, 7, 0, 1, 1);

        spinBox_row_height = new QSpinBox(groupBox_TrackTableView);
        spinBox_row_height->setObjectName("spinBox_row_height");
        spinBox_row_height->setMinimum(5);
        spinBox_row_height->setMaximum(100);
        spinBox_row_height->setValue(20);

        gridLayout_track_table_view->addWidget(spinBox_row_height, 7, 1, 1, 2);

        label_libraryFont = new QLabel(groupBox_TrackTableView);
        label_libraryFont->setObjectName("label_libraryFont");
        label_libraryFont->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignVCenter);

        gridLayout_track_table_view->addWidget(label_libraryFont, 8, 0, 1, 1);

        lineEdit_library_font = new QLineEdit(groupBox_TrackTableView);
        lineEdit_library_font->setObjectName("lineEdit_library_font");
        lineEdit_library_font->setReadOnly(true);

        gridLayout_track_table_view->addWidget(lineEdit_library_font, 8, 1, 1, 1);

        btn_library_font = new QToolButton(groupBox_TrackTableView);
        btn_library_font->setObjectName("btn_library_font");

        gridLayout_track_table_view->addWidget(btn_library_font, 8, 2, 1, 1);

        label_bpm_precision = new QLabel(groupBox_TrackTableView);
        label_bpm_precision->setObjectName("label_bpm_precision");
        label_bpm_precision->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignVCenter);

        gridLayout_track_table_view->addWidget(label_bpm_precision, 9, 0, 1, 1);

        spinbox_bpm_precision = new QSpinBox(groupBox_TrackTableView);
        spinbox_bpm_precision->setObjectName("spinbox_bpm_precision");

        gridLayout_track_table_view->addWidget(spinbox_bpm_precision, 9, 1, 1, 2);

        checkbox_played_track_color = new QCheckBox(groupBox_TrackTableView);
        checkbox_played_track_color->setObjectName("checkbox_played_track_color");

        gridLayout_track_table_view->addWidget(checkbox_played_track_color, 10, 0, 1, 3);

        verticalLayout->addWidget(groupBox_TrackTableView);

        groupBox_Search = new QGroupBox(DlgPrefLibraryDlg);
        groupBox_Search->setObjectName("groupBox_Search");
        gridLayout_search = new QGridLayout(groupBox_Search);
        gridLayout_search->setSpacing(6);
        gridLayout_search->setContentsMargins(11, 11, 11, 11);
        gridLayout_search->setObjectName("gridLayout_search");
        label_searchDebouncingTimeout = new QLabel(groupBox_Search);
        label_searchDebouncingTimeout->setObjectName("label_searchDebouncingTimeout");
        label_searchDebouncingTimeout->setAlignment(
                Qt::AlignLeading | Qt::AlignLeft | Qt::AlignVCenter);

        gridLayout_search->addWidget(label_searchDebouncingTimeout, 0, 0, 1, 1);

        spinBox_search_debouncing_timeout = new QSpinBox(groupBox_Search);
        spinBox_search_debouncing_timeout->setObjectName("spinBox_search_debouncing_timeout");

        gridLayout_search->addWidget(spinBox_search_debouncing_timeout, 0, 1, 1, 2);

        checkBox_enable_search_completions = new QCheckBox(groupBox_Search);
        checkBox_enable_search_completions->setObjectName("checkBox_enable_search_completions");

        gridLayout_search->addWidget(checkBox_enable_search_completions, 1, 0, 1, 3);

        checkBox_enable_search_history_shortcuts = new QCheckBox(groupBox_Search);
        checkBox_enable_search_history_shortcuts->setObjectName(
                "checkBox_enable_search_history_shortcuts");

        gridLayout_search->addWidget(checkBox_enable_search_history_shortcuts, 2, 0, 1, 3);

        label_searchBpmFuzzyRange = new QLabel(groupBox_Search);
        label_searchBpmFuzzyRange->setObjectName("label_searchBpmFuzzyRange");
        label_searchBpmFuzzyRange->setAlignment(
                Qt::AlignLeading | Qt::AlignLeft | Qt::AlignVCenter);

        gridLayout_search->addWidget(label_searchBpmFuzzyRange, 3, 0, 1, 1);

        comboBox_search_bpm_fuzzy_range = new QComboBox(groupBox_Search);
        comboBox_search_bpm_fuzzy_range->setObjectName("comboBox_search_bpm_fuzzy_range");

        gridLayout_search->addWidget(comboBox_search_bpm_fuzzy_range, 3, 1, 1, 2);

        label_searchBpmFuzzyRangeInfo = new QLabel(groupBox_Search);
        label_searchBpmFuzzyRangeInfo->setObjectName("label_searchBpmFuzzyRangeInfo");
        label_searchBpmFuzzyRangeInfo->setWordWrap(true);

        gridLayout_search->addWidget(label_searchBpmFuzzyRangeInfo, 4, 0, 1, 3);

        verticalLayout->addWidget(groupBox_Search);

        groupBox_History = new QGroupBox(DlgPrefLibraryDlg);
        groupBox_History->setObjectName("groupBox_History");
        gridLayout_history = new QGridLayout(groupBox_History);
        gridLayout_history->setSpacing(6);
        gridLayout_history->setContentsMargins(11, 11, 11, 11);
        gridLayout_history->setObjectName("gridLayout_history");
        label_history_track_duplicate_distance = new QLabel(groupBox_History);
        label_history_track_duplicate_distance->setObjectName(
                "label_history_track_duplicate_distance");
        label_history_track_duplicate_distance->setAlignment(
                Qt::AlignLeading | Qt::AlignLeft | Qt::AlignVCenter);
        label_history_track_duplicate_distance->setWordWrap(true);

        gridLayout_history->addWidget(label_history_track_duplicate_distance, 1, 0, 1, 1);

        spinbox_history_track_duplicate_distance = new QSpinBox(groupBox_History);
        spinbox_history_track_duplicate_distance->setObjectName(
                "spinbox_history_track_duplicate_distance");
        spinbox_history_track_duplicate_distance->setMinimum(0);
        spinbox_history_track_duplicate_distance->setMaximum(99);
        spinbox_history_track_duplicate_distance->setValue(1);

        gridLayout_history->addWidget(spinbox_history_track_duplicate_distance, 1, 1, 1, 2);

        label_history_cleanup = new QLabel(groupBox_History);
        label_history_cleanup->setObjectName("label_history_cleanup");
        label_history_cleanup->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignVCenter);

        gridLayout_history->addWidget(label_history_cleanup, 2, 0, 1, 1);

        spinbox_history_min_tracks_to_keep = new QSpinBox(groupBox_History);
        spinbox_history_min_tracks_to_keep->setObjectName("spinbox_history_min_tracks_to_keep");
        spinbox_history_min_tracks_to_keep->setMinimum(1);
        spinbox_history_min_tracks_to_keep->setMaximum(99);
        spinbox_history_min_tracks_to_keep->setValue(1);

        gridLayout_history->addWidget(spinbox_history_min_tracks_to_keep, 2, 1, 1, 2);

        verticalLayout->addWidget(groupBox_History);

        groupBox_GroupedCrates = new QGroupBox(DlgPrefLibraryDlg);
        groupBox_GroupedCrates->setObjectName("groupBox_GroupedCrates");
        groupBox_GroupedCrates->setMinimumSize(QSize(0, 100));
        checkBox_grouped_crates_enable = new QCheckBox(groupBox_GroupedCrates);
        checkBox_grouped_crates_enable->setObjectName("checkBox_grouped_crates_enable");
        checkBox_grouped_crates_enable->setGeometry(QRect(10, 20, 300, 20));
        radioButton_grouped_crates_fixed_length = new QRadioButton(groupBox_GroupedCrates);
        radioButton_grouped_crates_fixed_length->setObjectName(
                "radioButton_grouped_crates_fixed_length");
        radioButton_grouped_crates_fixed_length->setGeometry(QRect(50, 40, 400, 20));
        radioButton_grouped_crates_fixed_length->setMinimumSize(QSize(400, 0));
        radioButton_grouped_crates_var_mask = new QRadioButton(groupBox_GroupedCrates);
        radioButton_grouped_crates_var_mask->setObjectName("radioButton_grouped_crates_var_mask");
        radioButton_grouped_crates_var_mask->setGeometry(QRect(50, 70, 400, 20));
        radioButton_grouped_crates_var_mask->setMinimumSize(QSize(400, 0));
        spinBox_grouped_crates_fixed_length = new QSpinBox(groupBox_GroupedCrates);
        spinBox_grouped_crates_fixed_length->setObjectName("spinBox_grouped_crates_fixed_length");
        spinBox_grouped_crates_fixed_length->setGeometry(QRect(460, 40, 42, 22));
        spinBox_grouped_crates_fixed_length->setMaximum(20);
        lineEdit_grouped_crates_var_mask = new QLineEdit(groupBox_GroupedCrates);
        lineEdit_grouped_crates_var_mask->setObjectName("lineEdit_grouped_crates_var_mask");
        lineEdit_grouped_crates_var_mask->setGeometry(QRect(460, 70, 113, 20));

        verticalLayout->addWidget(groupBox_GroupedCrates);

        groupBox_external_libraries = new QGroupBox(DlgPrefLibraryDlg);
        groupBox_external_libraries->setObjectName("groupBox_external_libraries");
        gridLayout_external_libraries = new QGridLayout(groupBox_external_libraries);
        gridLayout_external_libraries->setSpacing(6);
        gridLayout_external_libraries->setContentsMargins(11, 11, 11, 11);
        gridLayout_external_libraries->setObjectName("gridLayout_external_libraries");
        checkBox_show_rhythmbox = new QCheckBox(groupBox_external_libraries);
        checkBox_show_rhythmbox->setObjectName("checkBox_show_rhythmbox");
        checkBox_show_rhythmbox->setChecked(true);

        gridLayout_external_libraries->addWidget(checkBox_show_rhythmbox, 0, 0, 1, 1);

        checkBox_show_banshee = new QCheckBox(groupBox_external_libraries);
        checkBox_show_banshee->setObjectName("checkBox_show_banshee");
        checkBox_show_banshee->setChecked(true);

        gridLayout_external_libraries->addWidget(checkBox_show_banshee, 1, 0, 1, 1);

        checkBox_show_itunes = new QCheckBox(groupBox_external_libraries);
        checkBox_show_itunes->setObjectName("checkBox_show_itunes");
        checkBox_show_itunes->setChecked(true);

        gridLayout_external_libraries->addWidget(checkBox_show_itunes, 2, 0, 1, 1);

        checkBox_show_traktor = new QCheckBox(groupBox_external_libraries);
        checkBox_show_traktor->setObjectName("checkBox_show_traktor");
        checkBox_show_traktor->setChecked(true);

        gridLayout_external_libraries->addWidget(checkBox_show_traktor, 3, 0, 1, 1);

        checkBox_show_rekordbox = new QCheckBox(groupBox_external_libraries);
        checkBox_show_rekordbox->setObjectName("checkBox_show_rekordbox");
        checkBox_show_rekordbox->setChecked(true);

        gridLayout_external_libraries->addWidget(checkBox_show_rekordbox, 4, 0, 1, 1);

        checkBox_show_serato = new QCheckBox(groupBox_external_libraries);
        checkBox_show_serato->setObjectName("checkBox_show_serato");
        checkBox_show_serato->setChecked(true);

        gridLayout_external_libraries->addWidget(checkBox_show_serato, 5, 0, 1, 1);

        label_4 = new QLabel(groupBox_external_libraries);
        label_4->setObjectName("label_4");
        label_4->setWordWrap(true);
        label_4->setContentsMargins(-1, 10, -1, -1);

        gridLayout_external_libraries->addWidget(label_4, 6, 0, 1, 1);

        label_11 = new QLabel(groupBox_external_libraries);
        label_11->setObjectName("label_11");
        label_11->setWordWrap(true);

        gridLayout_external_libraries->addWidget(label_11, 7, 0, 1, 1);

        verticalLayout->addWidget(groupBox_external_libraries);

        groupBox_cover_art_fetcher = new QGroupBox(DlgPrefLibraryDlg);
        groupBox_cover_art_fetcher->setObjectName("groupBox_cover_art_fetcher");
        gridLayout_cover_art_fetcher = new QGridLayout(groupBox_cover_art_fetcher);
        gridLayout_cover_art_fetcher->setSpacing(6);
        gridLayout_cover_art_fetcher->setContentsMargins(11, 11, 11, 11);
        gridLayout_cover_art_fetcher->setObjectName("gridLayout_cover_art_fetcher");
        label_12 = new QLabel(groupBox_cover_art_fetcher);
        label_12->setObjectName("label_12");
        label_12->setWordWrap(true);

        gridLayout_cover_art_fetcher->addWidget(label_12, 0, 0, 1, 1);

        label_13 = new QLabel(groupBox_cover_art_fetcher);
        label_13->setObjectName("label_13");
        label_13->setWordWrap(true);

        gridLayout_cover_art_fetcher->addWidget(label_13, 1, 0, 1, 1);

        radioButton_cover_art_fetcher_highest = new QRadioButton(groupBox_cover_art_fetcher);
        radioButton_cover_art_fetcher_highest->setObjectName(
                "radioButton_cover_art_fetcher_highest");
        radioButton_cover_art_fetcher_highest->setChecked(false);

        gridLayout_cover_art_fetcher->addWidget(radioButton_cover_art_fetcher_highest, 2, 0, 1, 1);

        radioButton_cover_art_fetcher_high = new QRadioButton(groupBox_cover_art_fetcher);
        radioButton_cover_art_fetcher_high->setObjectName("radioButton_cover_art_fetcher_high");
        radioButton_cover_art_fetcher_high->setChecked(false);

        gridLayout_cover_art_fetcher->addWidget(radioButton_cover_art_fetcher_high, 3, 0, 1, 1);

        radioButton_cover_art_fetcher_medium = new QRadioButton(groupBox_cover_art_fetcher);
        radioButton_cover_art_fetcher_medium->setObjectName("radioButton_cover_art_fetcher_medium");
        radioButton_cover_art_fetcher_medium->setChecked(false);

        gridLayout_cover_art_fetcher->addWidget(radioButton_cover_art_fetcher_medium, 4, 0, 1, 1);

        radioButton_cover_art_fetcher_lowest = new QRadioButton(groupBox_cover_art_fetcher);
        radioButton_cover_art_fetcher_lowest->setObjectName("radioButton_cover_art_fetcher_lowest");
        radioButton_cover_art_fetcher_lowest->setChecked(false);

        gridLayout_cover_art_fetcher->addWidget(radioButton_cover_art_fetcher_lowest, 5, 0, 1, 1);

        verticalLayout->addWidget(groupBox_cover_art_fetcher);

        groupBox_settingsDir = new QGroupBox(DlgPrefLibraryDlg);
        groupBox_settingsDir->setObjectName("groupBox_settingsDir");
        vlayout_settingsDir = new QVBoxLayout(groupBox_settingsDir);
        vlayout_settingsDir->setSpacing(6);
        vlayout_settingsDir->setContentsMargins(11, 11, 11, 11);
        vlayout_settingsDir->setObjectName("vlayout_settingsDir");
        label_settingsDir = new QLabel(groupBox_settingsDir);
        label_settingsDir->setObjectName("label_settingsDir");
        label_settingsDir->setWordWrap(true);

        vlayout_settingsDir->addWidget(label_settingsDir);

        label_settingsManualLink = new QLabel(groupBox_settingsDir);
        label_settingsManualLink->setObjectName("label_settingsManualLink");
        label_settingsManualLink->setWordWrap(true);
        label_settingsManualLink->setOpenExternalLinks(true);

        vlayout_settingsDir->addWidget(label_settingsManualLink);

        label_settingsWarning = new QLabel(groupBox_settingsDir);
        label_settingsWarning->setObjectName("label_settingsWarning");
        label_settingsWarning->setWordWrap(true);

        vlayout_settingsDir->addWidget(label_settingsWarning);

        pushButton_open_settings_dir = new QPushButton(groupBox_settingsDir);
        pushButton_open_settings_dir->setObjectName("pushButton_open_settings_dir");
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(
                pushButton_open_settings_dir->sizePolicy().hasHeightForWidth());
        pushButton_open_settings_dir->setSizePolicy(sizePolicy);

        vlayout_settingsDir->addWidget(pushButton_open_settings_dir);

        verticalLayout->addWidget(groupBox_settingsDir);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

#if QT_CONFIG(shortcut)
        label_searchBpmFuzzyRange->setBuddy(comboBox_search_bpm_fuzzy_range);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(dirList, pushButton_add_dir);
        QWidget::setTabOrder(pushButton_add_dir, pushButton_relocate_dir);
        QWidget::setTabOrder(pushButton_relocate_dir, pushButton_remove_dir);
        QWidget::setTabOrder(pushButton_remove_dir, checkBox_library_scan);
        QWidget::setTabOrder(checkBox_library_scan, checkBox_sync_track_metadata);
        QWidget::setTabOrder(checkBox_sync_track_metadata, checkBox_serato_metadata_export);
        QWidget::setTabOrder(checkBox_serato_metadata_export,
                checkBox_edit_metadata_selected_clicked);
        QWidget::setTabOrder(checkBox_edit_metadata_selected_clicked, radioButton_dbclick_deck);
        QWidget::setTabOrder(radioButton_dbclick_deck, radioButton_dbclick_bottom);
        QWidget::setTabOrder(radioButton_dbclick_bottom, radioButton_dbclick_top);
        QWidget::setTabOrder(radioButton_dbclick_top, radioButton_dbclick_ignore);
        QWidget::setTabOrder(radioButton_dbclick_ignore, spinbox_bpm_precision);
        QWidget::setTabOrder(spinbox_bpm_precision, checkbox_played_track_color);
        QWidget::setTabOrder(checkbox_played_track_color, spinbox_history_track_duplicate_distance);
        QWidget::setTabOrder(spinbox_history_track_duplicate_distance,
                spinbox_history_min_tracks_to_keep);
        QWidget::setTabOrder(spinbox_history_min_tracks_to_keep, checkBox_use_relative_path);
        QWidget::setTabOrder(checkBox_use_relative_path, spinBox_row_height);
        QWidget::setTabOrder(spinBox_row_height, btn_library_font);
        QWidget::setTabOrder(btn_library_font, spinBox_search_debouncing_timeout);
        QWidget::setTabOrder(spinBox_search_debouncing_timeout, checkBox_enable_search_completions);
        QWidget::setTabOrder(checkBox_enable_search_completions,
                checkBox_enable_search_history_shortcuts);
        QWidget::setTabOrder(checkBox_enable_search_history_shortcuts,
                comboBox_search_bpm_fuzzy_range);
        QWidget::setTabOrder(comboBox_search_bpm_fuzzy_range, checkBox_show_rhythmbox);
        QWidget::setTabOrder(checkBox_show_rhythmbox, checkBox_show_banshee);
        QWidget::setTabOrder(checkBox_show_banshee, checkBox_show_itunes);
        QWidget::setTabOrder(checkBox_show_itunes, checkBox_show_traktor);
        QWidget::setTabOrder(checkBox_show_traktor, checkBox_show_rekordbox);
        QWidget::setTabOrder(checkBox_show_rekordbox, checkBox_show_serato);
        QWidget::setTabOrder(checkBox_show_serato, radioButton_cover_art_fetcher_highest);
        QWidget::setTabOrder(radioButton_cover_art_fetcher_highest,
                radioButton_cover_art_fetcher_high);
        QWidget::setTabOrder(radioButton_cover_art_fetcher_high,
                radioButton_cover_art_fetcher_medium);
        QWidget::setTabOrder(radioButton_cover_art_fetcher_medium,
                radioButton_cover_art_fetcher_lowest);
        QWidget::setTabOrder(radioButton_cover_art_fetcher_lowest, pushButton_open_settings_dir);

        retranslateUi(DlgPrefLibraryDlg);

        QMetaObject::connectSlotsByName(DlgPrefLibraryDlg);
    } // setupUi

    void retranslateUi(QWidget* DlgPrefLibraryDlg) {
        groupBox_MusicDirectories->setTitle(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Music Directories", nullptr));
#if QT_CONFIG(tooltip)
        pushButton_add_dir->setToolTip(QCoreApplication::translate(
                "DlgPrefLibraryDlg",
                "Add a directory where your music is stored. Mixxx will watch "
                "this directory and its subdirectories for new tracks.",
                nullptr));
#endif // QT_CONFIG(tooltip)
        pushButton_add_dir->setText(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Add", nullptr));
#if QT_CONFIG(tooltip)
        pushButton_relocate_dir->setToolTip(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "If an existing music directory is moved, Mixxx "
                        "doesn't know where to find the audio files in it. "
                        "Choose Relink to select the music directory in its "
                        "new location. <br/> This will re-establish the links "
                        "to the audio files in the Mixxx library.",
                        nullptr));
#endif // QT_CONFIG(tooltip)
        pushButton_relocate_dir->setText(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Relink", nullptr));
#if QT_CONFIG(tooltip)
        pushButton_remove_dir->setToolTip(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "If removed, Mixxx will no longer watch this directory "
                        "and its subdirectories for new tracks.",
                        nullptr));
#endif // QT_CONFIG(tooltip)
        pushButton_remove_dir->setText(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Remove", nullptr));
        checkBox_library_scan->setText(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "Rescan directories on start-up",
                        nullptr));
        groupBox_AudioFileFormats->setTitle(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Audio File Formats", nullptr));
        builtInFormats->setText(QString());
        groupBox_AudioFileTags->setTitle(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "Track Metadata Synchronization / Playlists",
                        nullptr));
#if QT_CONFIG(tooltip)
        checkBox_sync_track_metadata->setToolTip(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "Automatically write modified track metadata from the "
                        "library into file tags and reimport metadata from "
                        "updated file tags into the library",
                        nullptr));
#endif // QT_CONFIG(tooltip)
        checkBox_sync_track_metadata->setText(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "Synchronize library track metadata from/to file tags",
                        nullptr));
#if QT_CONFIG(tooltip)
        checkBox_serato_metadata_export->setToolTip(QCoreApplication::translate(
                "DlgPrefLibraryDlg",
                "Keeps track color, beat grid, bpm lock, cue points, and loops "
                "synchronized with SERATO_MARKERS/MARKERS2 file "
                "tags.<br/><br/>WARNING: Enabling this option also enables the "
                "reimport of Serato metadata after files have been modified "
                "outside of Mixxx. On reimport existing metadata in Mixxx is "
                "replaced with the metadata found in file tags. Custom "
                "metadata not included in file tags like loop colors is lost.",
                nullptr));
#endif // QT_CONFIG(tooltip)
        checkBox_serato_metadata_export->setText(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "Synchronize Serato track metadata from/to file tags "
                        "(experimental)",
                        nullptr));
        checkBox_use_relative_path->setText(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "Use relative paths for playlist export if possible",
                        nullptr));
        groupBox_TrackTableView->setTitle(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Track Table View", nullptr));
        checkBox_edit_metadata_selected_clicked->setText(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "Edit metadata after clicking selected track",
                        nullptr));
        label_doubeClickAction->setText(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Track Double-Click Action:", nullptr));
        radioButton_dbclick_deck->setText(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "Load track to next available deck",
                        nullptr));
        radioButton_dbclick_bottom->setText(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "Add track to Auto DJ queue (bottom)",
                        nullptr));
        radioButton_dbclick_top->setText(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "Add track to Auto DJ queue (top)",
                        nullptr));
        radioButton_dbclick_ignore->setText(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Ignore", nullptr));
        label_rowHeight->setText(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Library Row Height:", nullptr));
        spinBox_row_height->setSuffix(QCoreApplication::translate(
                "DlgPrefLibraryDlg", " px", nullptr));
        label_libraryFont->setText(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Library Font:", nullptr));
        btn_library_font->setText(QCoreApplication::translate("DlgPrefLibraryDlg", "...", nullptr));
        label_bpm_precision->setText(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "BPM display precision:", nullptr));
        checkbox_played_track_color->setText(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Grey out played tracks", nullptr));
        groupBox_Search->setTitle(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Track Search", nullptr));
        label_searchDebouncingTimeout->setText(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Search-as-you-type timeout:", nullptr));
        spinBox_search_debouncing_timeout->setSuffix(
                QCoreApplication::translate(
                        "DlgPrefLibraryDlg", " ms", nullptr));
        checkBox_enable_search_completions->setText(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Enable search completions", nullptr));
        checkBox_enable_search_history_shortcuts->setText(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "Enable search history keyboard shortcuts",
                        nullptr));
        label_searchBpmFuzzyRange->setText(QCoreApplication::translate(
                "DlgPrefLibraryDlg",
                "Percentage of pitch slider range for 'fuzzy' BPM search:",
                nullptr));
        label_searchBpmFuzzyRangeInfo->setText(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "This range will be used for the 'fuzzy' BPM search "
                        "(~bpm:) via the search box, as well as for BPM search "
                        "in Track context menu > Search related Tracks",
                        nullptr));
        groupBox_History->setTitle(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Session History", nullptr));
        label_history_track_duplicate_distance->setText(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "Track duplicate distance",
                        nullptr));
#if QT_CONFIG(tooltip)
        spinbox_history_track_duplicate_distance->setToolTip(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "When playing a track again log it to the session "
                        "history only if more than N other tracks have been "
                        "played in the meantime",
                        nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        label_history_cleanup->setToolTip(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "History playlist with less than N tracks will be "
                        "deleted<br/><br/>Note: the cleanup will be performed "
                        "during startup and shutdown of Mixxx.",
                        nullptr));
#endif // QT_CONFIG(tooltip)
        label_history_cleanup->setText(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "Delete history playlist with less than N tracks",
                        nullptr));
        groupBox_GroupedCrates->setTitle(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Grouped Crates", nullptr));
        checkBox_grouped_crates_enable->setText(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "Enable Grouped Crates (Mixxx restart needed)",
                        nullptr));
        radioButton_grouped_crates_fixed_length->setText(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "Fixed length: 1 level of grouping: Group = "
                        "#Characters",
                        nullptr));
        radioButton_grouped_crates_var_mask->setText(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "Variable length, multilevel: grouping: Groups are "
                        "delimited with a mask: ",
                        nullptr));
        groupBox_external_libraries->setTitle(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "External Libraries", nullptr));
        checkBox_show_rhythmbox->setText(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Show Rhythmbox Library", nullptr));
        checkBox_show_banshee->setText(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Show Banshee Library", nullptr));
        checkBox_show_itunes->setText(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Show iTunes Library", nullptr));
        checkBox_show_traktor->setText(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Show Traktor Library", nullptr));
        checkBox_show_rekordbox->setText(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Show Rekordbox Library", nullptr));
        checkBox_show_serato->setText(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Show Serato Library", nullptr));
        label_4->setText(QCoreApplication::translate("DlgPrefLibraryDlg",
                "All external libraries shown are write protected.",
                nullptr));
        label_11->setText(QCoreApplication::translate("DlgPrefLibraryDlg",
                "You will need to restart Mixxx for these settings to take "
                "effect.",
                nullptr));
        groupBox_cover_art_fetcher->setTitle(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "Preferred Cover Art Fetcher Resolution",
                        nullptr));
        label_12->setText(QCoreApplication::translate("DlgPrefLibraryDlg",
                "Fetch cover art from coverartarchive.com by using Import "
                "Metadata From Musicbrainz.",
                nullptr));
        label_13->setText(QCoreApplication::translate("DlgPrefLibraryDlg",
                "Note: \">1200 px\" can fetch up to very large cover arts.",
                nullptr));
        radioButton_cover_art_fetcher_highest->setText(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        ">1200 px (if available)",
                        nullptr));
        radioButton_cover_art_fetcher_high->setText(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "1200 px (if available)", nullptr));
        radioButton_cover_art_fetcher_medium->setText(
                QCoreApplication::translate(
                        "DlgPrefLibraryDlg", "500 px", nullptr));
        radioButton_cover_art_fetcher_lowest->setText(
                QCoreApplication::translate(
                        "DlgPrefLibraryDlg", "250 px", nullptr));
        groupBox_settingsDir->setTitle(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Settings Directory", nullptr));
        label_settingsDir->setText(QCoreApplication::translate(
                "DlgPrefLibraryDlg",
                "The Mixxx settings directory contains the library database, "
                "various configuration files, log files, track analysis data, "
                "as well as custom controller mappings.",
                nullptr));
        label_settingsManualLink->setText(QString());
        label_settingsWarning->setText(
                QCoreApplication::translate("DlgPrefLibraryDlg",
                        "Edit those files only if you know what you are doing "
                        "and only while Mixxx is not running.",
                        nullptr));
        pushButton_open_settings_dir->setText(QCoreApplication::translate(
                "DlgPrefLibraryDlg", "Open Mixxx Settings Folder", nullptr));
        (void)DlgPrefLibraryDlg;
    } // retranslateUi
};

namespace Ui {
class DlgPrefLibraryDlg : public Ui_DlgPrefLibraryDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGPREFLIBRARYDLG_H
