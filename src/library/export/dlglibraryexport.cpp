#include "library/export/dlglibraryexport.h"

#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStandardPaths>
#include <djinterop/djinterop.hpp>

#include "library/export/engineprimeexportrequest.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackset/crate/crate.h"
#include "library/trackset/crate/crateid.h"
#include "library/trackset/crate/cratestorage.h"
#include "moc_dlglibraryexport.cpp"

namespace e = djinterop::engine;

namespace mixxx {

namespace {
const QString kDefaultMixxxExportDirName = QStringLiteral("mixxx-export");
const QString kLastDirConfigItemName = QStringLiteral("LastLibraryExportDirectory");

void populateCrates(
        QListWidget* pListWidget,
        const TrackCollection& trackCollection) {
    // Populate list of crates.
    CrateSelectResult crates = trackCollection.crates().selectCrates();
    Crate crate;
    pListWidget->clear();
    while (crates.populateNext(&crate)) {
        auto pItem = std::make_unique<QListWidgetItem>(crate.getName());
        pItem->setData(Qt::UserRole, crate.getId().toVariant());
        pListWidget->addItem(pItem.release());
    }
}

void populatePlaylists(
        QListWidget* pListWidget,
        TrackCollection& trackCollection) {
    const auto playlistIdAndNamePairs =
            trackCollection.getPlaylistDAO().getPlaylists(
                    PlaylistDAO::PLHT_NOT_HIDDEN);
    pListWidget->clear();
    for (const auto& [playlistId, playlistName] : playlistIdAndNamePairs) {
        auto pItem = std::make_unique<QListWidgetItem>(playlistName);
        pItem->setData(Qt::UserRole, playlistId);
        pListWidget->addItem(pItem.release());
    }
}
} // namespace

DlgLibraryExport::DlgLibraryExport(
        QWidget* parent,
        UserSettingsPointer pConfig,
        TrackCollectionManager* pTrackCollectionManager)
        : QDialog(parent),
          m_pConfig{pConfig},
          m_pTrackCollectionManager{pTrackCollectionManager} {
    // Selectable list of crates from the Mixxx library.
    auto pCratesLabel = make_parented<QLabel>(tr("Crates"), this);
    m_pCratesList = make_parented<QListWidget>(this);
    m_pCratesList->setSelectionMode(QListWidget::ExtendedSelection);

    // Selectable list of playlists from the Mixxx library.
    auto pPlaylistsLabel = make_parented<QLabel>(tr("Playlists"), this);
    m_pPlaylistsList = make_parented<QListWidget>(this);
    m_pPlaylistsList->setSelectionMode(QListWidget::ExtendedSelection);

    // Read-only text fields showing key directories for export.
    m_pExportDirectoryTextField = make_parented<QLineEdit>(this);
    m_pExportDirectoryTextField->setReadOnly(true);

    // Remember the last export directory, or use documents as a fallback.
    QString fallbackExportDirectory =
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString lastExportDirectory =
            m_pConfig->getValue(ConfigKey("[Library]", kLastDirConfigItemName),
                    fallbackExportDirectory);
    if (!QDir{lastExportDirectory}.exists()) {
        lastExportDirectory = fallbackExportDirectory;
    }

    m_pExportDirectoryTextField->setText(lastExportDirectory);

    m_pVersionCombo = make_parented<QComboBox>(this);
    m_pVersionCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    m_pExistingDatabaseLabel = make_parented<QLabel>(this);
    m_pExistingDatabaseLabel->setWordWrap(true);

    // Radio buttons to allow choice between exporting the whole music library
    // or just tracks in a selection of crates.
    m_pWholeLibraryRadio = make_parented<QRadioButton>(tr("Entire music library"), this);
    m_pWholeLibraryRadio->setChecked(true);
    m_pCratesList->setEnabled(false);
    connect(m_pWholeLibraryRadio,
            &QRadioButton::clicked,
            this,
            [this]() { m_pCratesList->setEnabled(false); m_pPlaylistsList->setEnabled(false); });
    m_pCratesAndPlaylistsRadio = make_parented<QRadioButton>(tr("Selected crates/playlists"), this);
    connect(m_pCratesAndPlaylistsRadio,
            &QRadioButton::clicked,
            this,
            [this]() { m_pCratesList->setEnabled(true); m_pPlaylistsList->setEnabled(true); });

    // Button to allow ability to browse for the export directory.
    auto pExportDirBrowseButton = make_parented<QPushButton>(tr("Browse"), this);
    connect(pExportDirBrowseButton,
            &QPushButton::clicked,
            this,
            &DlgLibraryExport::browseExportDirectory);
    auto pExportDirLayout = std::make_unique<QHBoxLayout>();
    pExportDirLayout->addWidget(m_pExportDirectoryTextField);
    pExportDirLayout->addWidget(pExportDirBrowseButton);

    auto pFormLayout = std::make_unique<QFormLayout>();
    pFormLayout->addRow(tr("Export directory"), pExportDirLayout.release());
    pFormLayout->addRow(tr("Database version"), m_pVersionCombo);
    pFormLayout->addRow(m_pExistingDatabaseLabel);

    // Buttons to begin the export or cancel.
    auto pExportButton = make_parented<QPushButton>(tr("Export"), this);
    pExportButton->setDefault(true);
    connect(pExportButton, &QPushButton::clicked, this, &DlgLibraryExport::exportRequested);
    auto pCancelButton = make_parented<QPushButton>(tr("Cancel"), this);
    connect(pCancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // Arrange action buttons at bottom of dialog.
    auto pButtonBarLayout = std::make_unique<QHBoxLayout>();
    pButtonBarLayout->addStretch(1);
    pButtonBarLayout->addWidget(pExportButton);
    pButtonBarLayout->addWidget(pCancelButton);

    // Visual representation of grid layout:
    //            0                    1                     2
    //   +---------------------------------------+----------------------+
    // 0 | <Whole library radio> (colspan=2)     | <Dir/version fields> |
    //   +-------------------+-------------------+      (rowspan=4)     |
    // 1 | <Crates/playlists radio> (colspan=2)  |                      |
    //   +-------------------+-------------------+                      |
    // 2 | <Crates label>    | <Playlists label> |                      |
    //   +-------------------+-------------------+                      |
    //   | <Crates list>     | <Playlists list>  |                      |
    // 3 |                   |                   |                      |
    //   |                   |                   |                      |
    //   +-------------------+-------------------+----------------------+
    // 4 |                                 <Action buttons> (colspan=3) |
    //   +--------------------------------------------------------------+
    auto pLayout = make_parented<QGridLayout>(this);
    pLayout->setColumnStretch(0, 1);
    pLayout->setColumnStretch(1, 1);
    pLayout->setColumnStretch(2, 2);
    pLayout->addWidget(m_pWholeLibraryRadio, 0, 0, 1, 2);
    pLayout->addWidget(m_pCratesAndPlaylistsRadio, 1, 0, 1, 2);
    pLayout->addWidget(pCratesLabel, 2, 0);
    pLayout->addWidget(pPlaylistsLabel, 2, 1);
    pLayout->addWidget(m_pCratesList, 3, 0);
    pLayout->addWidget(m_pPlaylistsList, 3, 1);
    pLayout->addLayout(pFormLayout.release(), 0, 2, 4, 1);
    pLayout->addLayout(pButtonBarLayout.release(), 4, 0, 1, 3);

    setLayout(pLayout);
    //: "Engine DJ" must not be translated
    setWindowTitle(tr("Export Library to Engine DJ"));

    show();
    raise();
    activateWindow();
}

void DlgLibraryExport::refresh() {
    // Refresh the list of crates and playlists.
    populateCrates(m_pCratesList, *m_pTrackCollectionManager->internalCollection());
    populatePlaylists(m_pPlaylistsList, *m_pTrackCollectionManager->internalCollection());

    // Check whether a database already exists in the specified directory.
    checkExistingDatabase();
}

void DlgLibraryExport::setInitialSelection(
        std::optional<CrateId> crateId, std::optional<int> playlistId) {
    if (!crateId && !playlistId) {
        m_pWholeLibraryRadio->setChecked(true);
        m_pCratesList->setEnabled(false);
        m_pPlaylistsList->setEnabled(false);
        return;
    }

    m_pCratesAndPlaylistsRadio->setChecked(true);
    m_pCratesList->setEnabled(true);
    m_pPlaylistsList->setEnabled(true);

    if (crateId) {
        for (auto i = 0; i < m_pCratesList->count(); ++i) {
            auto* pItem = m_pCratesList->item(i);
            const auto currCrateId = CrateId(pItem->data(Qt::UserRole));
            if (currCrateId == crateId) {
                m_pCratesList->setCurrentItem(pItem);
                return;
            }
        }
    }

    if (playlistId) {
        for (auto i = 0; i < m_pPlaylistsList->count(); ++i) {
            auto* pItem = m_pPlaylistsList->item(i);
            const auto currPlaylistId = pItem->data(Qt::UserRole).toInt();
            if (currPlaylistId == playlistId) {
                m_pPlaylistsList->setCurrentItem(pItem);
                return;
            }
        }
    }
}

void DlgLibraryExport::browseExportDirectory() {
    QString lastExportDirectory =
            m_pConfig->getValue(ConfigKey("[Library]", kLastDirConfigItemName),
                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    const auto exportDirectory = QFileDialog::getExistingDirectory(
            nullptr, tr("Export Library To"), lastExportDirectory);
    if (exportDirectory.isEmpty()) {
        return;
    }
    m_pConfig->set(
            ConfigKey("[Library]", kLastDirConfigItemName), ConfigValue(exportDirectory));

    m_pExportDirectoryTextField->setText(exportDirectory);

    // Check if there is an existing database in the given directory.
    checkExistingDatabase();
}

void DlgLibraryExport::exportRequested() {
    // Check a base export directory has been chosen
    if (m_pExportDirectoryTextField->text().trimmed().isEmpty()) {
        QMessageBox::information(this,
                tr("No Export Directory Chosen"),
                tr("No export directory was chosen. Please choose a directory "
                   "in order to export the music library."),
                QMessageBox::Ok,
                QMessageBox::Ok);
        return;
    }

    QDir baseExportDirectory{m_pExportDirectoryTextField->text()};
    const auto databaseDirectory = baseExportDirectory.filePath(
            e::default_database_dir_name);
    const auto musicDirectory = baseExportDirectory.filePath(kDefaultMixxxExportDirName);

    // Work out what version was requested.
    // If there is an existing database, the version does not matter.
    int versionIndex = m_pVersionCombo->currentData().toInt();
    e::engine_schema exportSchemaVersion =
            versionIndex == -1 ? e::latest_schema : e::supported_schemas[versionIndex];

    // Construct a request to export the library/crates/playlists.
    auto pRequest = QSharedPointer<EnginePrimeExportRequest>::create();
    pRequest->engineLibraryDbDir.setPath(databaseDirectory);
    pRequest->musicFilesDir.setPath(musicDirectory);
    pRequest->exportSchemaVersion = exportSchemaVersion;
    if (m_pCratesList->isEnabled()) {
        const auto selectedItems = m_pCratesList->selectedItems();
        for (auto* pItem : selectedItems) {
            CrateId id{pItem->data(Qt::UserRole)};
            pRequest->crateIdsToExport.insert(id);
        }
    }
    if (m_pPlaylistsList->isEnabled()) {
        const auto selectedItems = m_pPlaylistsList->selectedItems();
        for (auto* pItem : selectedItems) {
            int id = pItem->data(Qt::UserRole).toInt();
            pRequest->playlistIdsToExport.insert(id);
        }
    }

    emit startEnginePrimeExport(pRequest);
    accept();
}

void DlgLibraryExport::checkExistingDatabase() {
    QDir baseExportDirectory{m_pExportDirectoryTextField->text()};
    const auto databaseDirectory = baseExportDirectory.filePath(
            e::default_database_dir_name);

    try {
        // See if an EL DB exists in the chosen dir already.
        bool exists = e::database_exists(databaseDirectory.toStdString());
        if (!exists) {
            // The user can freely choose a schema version for their new database.
            m_pExistingDatabaseLabel->setText("");
            m_pVersionCombo->clear();
            m_pVersionCombo->setEnabled(true);
            for (int versionIndex = 0;
                    versionIndex < static_cast<int>(e::supported_schemas.size());
                    ++versionIndex) {
                e::engine_schema schemaVersion = e::supported_schemas[versionIndex];
                m_pVersionCombo->insertItem(0,
                        QString::fromStdString(to_application_version_string(schemaVersion)),
                        QVariant{versionIndex});
                if (schemaVersion == e::latest_schema) {
                    // Latest firmware version is the default selection.
                    m_pVersionCombo->setCurrentIndex(0);
                }
            }
            return;
        }

        // Load the existing database, and set the displayed version widget
        // accordingly.  Changing the schema version of existing databases is
        // not currently supported.
        djinterop::database db = e::load_database(databaseDirectory.toStdString());
        m_pExistingDatabaseLabel->setText(
                tr("A database already exists in the chosen directory. "
                   "Exported tracks will be added into this database."));
        m_pVersionCombo->clear();
        m_pVersionCombo->insertItem(
                0, QString::fromStdString(db.version_name()), QVariant{-1});
        m_pVersionCombo->setEnabled(false);

    } catch (std::exception& e) {
        Q_UNUSED(e);
        m_pExistingDatabaseLabel->setText(
                tr("A database already exists in the chosen directory, "
                   "but there was a problem loading it. Export is not "
                   "guaranteed to succeed in this situation."));
        m_pVersionCombo->clear();
        m_pVersionCombo->setEnabled(false);
    }
}

} // namespace mixxx
