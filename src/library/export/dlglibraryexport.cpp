#include "library/export/dlglibraryexport.h"

#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStandardPaths>
#include <algorithm>
#include <djinterop/djinterop.hpp>
#include <string>

#include "library/export/engineprimeexportrequest.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackset/crate/crateid.h"
#include "library/trackset/crate/cratestorage.h"

namespace el = djinterop::enginelibrary;

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
        pItem->setData(Qt::UserRole, crate.getId().value());
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
    m_pCratesList = make_parented<QListWidget>();
    m_pCratesList->setSelectionMode(QListWidget::ExtendedSelection);

    // Read-only text fields showing key directories for export.
    m_pExportDirectoryTextField = make_parented<QLineEdit>();
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

    m_pVersionCombo = make_parented<QComboBox>();
    m_pVersionCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    m_pExistingDatabaseLabel = make_parented<QLabel>();
    m_pExistingDatabaseLabel->setWordWrap(true);

    // Radio buttons to allow choice between exporting the whole music library
    // or just tracks in a selection of crates.
    m_pWholeLibraryRadio = make_parented<QRadioButton>(tr("Entire music library"));
    m_pWholeLibraryRadio->setChecked(true);
    m_pCratesList->setEnabled(false);
    connect(m_pWholeLibraryRadio,
            &QRadioButton::clicked,
            this,
            [this]() { m_pCratesList->setEnabled(false); });
    m_pCratesRadio = make_parented<QRadioButton>(tr("Selected crates"));
    connect(m_pCratesRadio,
            &QRadioButton::clicked,
            this,
            [this]() { m_pCratesList->setEnabled(true); });

    // Button to allow ability to browse for the export directory.
    auto pExportDirBrowseButton = make_parented<QPushButton>(tr("Browse"));
    connect(pExportDirBrowseButton,
            &QPushButton::clicked,
            this,
            &DlgLibraryExport::browseExportDirectory);
    auto pExportDirLayout = make_parented<QHBoxLayout>();
    pExportDirLayout->addWidget(m_pExportDirectoryTextField);
    pExportDirLayout->addWidget(pExportDirBrowseButton);

    auto pFormLayout = make_parented<QFormLayout>();
    pFormLayout->addRow(tr("Export directory"), pExportDirLayout);
    pFormLayout->addRow(tr("Database version"), m_pVersionCombo);
    pFormLayout->addRow(m_pExistingDatabaseLabel);

    // Buttons to begin the export or cancel.
    auto pExportButton = make_parented<QPushButton>(tr("Export"));
    pExportButton->setDefault(true);
    connect(pExportButton, &QPushButton::clicked, this, &DlgLibraryExport::exportRequested);
    auto pCancelButton = make_parented<QPushButton>(tr("Cancel"));
    connect(pCancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // Arrange action buttons at bottom of dialog.
    auto pButtonBarLayout = make_parented<QHBoxLayout>();
    pButtonBarLayout->addStretch(1);
    pButtonBarLayout->addWidget(pExportButton);
    pButtonBarLayout->addWidget(pCancelButton);

    auto pLayout = make_parented<QGridLayout>();
    pLayout->setColumnStretch(0, 1);
    pLayout->setColumnStretch(1, 2);
    pLayout->addWidget(m_pWholeLibraryRadio, 0, 0);
    pLayout->addWidget(m_pCratesRadio, 1, 0);
    pLayout->addWidget(m_pCratesList, 2, 0);
    pLayout->addLayout(pFormLayout, 0, 1, 3, 1);
    pLayout->addLayout(pButtonBarLayout, 3, 0, 1, 2);

    setLayout(pLayout);
    setWindowTitle(tr("Export Library to Engine Prime"));

    show();
    raise();
    activateWindow();
}

void DlgLibraryExport::refresh() {
    // Refresh the list of crates.
    populateCrates(m_pCratesList, *m_pTrackCollectionManager->internalCollection());

    // Check whether a database already exists in the specified directory.
    checkExistingDatabase();
}

void DlgLibraryExport::setSelectedCrate(std::optional<CrateId> crateId) {
    if (!crateId) {
        m_pWholeLibraryRadio->setChecked(true);
        m_pCratesList->setEnabled(false);
        return;
    }

    m_pCratesRadio->setChecked(true);
    m_pCratesList->setEnabled(true);
    for (auto i = 0; i < m_pCratesList->count(); ++i) {
        auto* pItem = m_pCratesList->item(i);
        const auto currCrateId = pItem->data(Qt::UserRole).toInt();
        if (currCrateId == crateId->value()) {
            m_pCratesList->setCurrentItem(pItem);
            return;
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
            el::default_database_dir_name);
    const auto musicDirectory = baseExportDirectory.filePath(kDefaultMixxxExportDirName);

    // Work out what version was requested.
    // If there is an existing database, the version does not matter.
    int versionIndex = m_pVersionCombo->currentData().toInt();
    djinterop::semantic_version exportVersion =
            versionIndex == -1 ? el::version_latest_firmware : el::all_versions[versionIndex];

    // Construct a request to export the library/crates.
    auto pRequest = QSharedPointer<EnginePrimeExportRequest>::create();
    pRequest->engineLibraryDbDir = QDir{databaseDirectory};
    pRequest->musicFilesDir = QDir{musicDirectory};
    pRequest->exportVersion = exportVersion;
    if (m_pCratesList->isEnabled()) {
        const auto selectedItems = m_pCratesList->selectedItems();
        for (auto* pItem : selectedItems) {
            CrateId id{pItem->data(Qt::UserRole).value<int>()};
            pRequest->crateIdsToExport.insert(id);
        }
    }

    emit startEnginePrimeExport(pRequest);
    accept();
}

void DlgLibraryExport::checkExistingDatabase() {
    QDir baseExportDirectory{m_pExportDirectoryTextField->text()};
    const auto databaseDirectory = baseExportDirectory.filePath(
            el::default_database_dir_name);

    try {
        // See if an EL DB exists in the chosen dir already.
        bool exists = el::database_exists(databaseDirectory.toStdString());
        if (!exists) {
            // The user can freely choose a schema version for their new database.
            m_pExistingDatabaseLabel->setText("");
            m_pVersionCombo->clear();
            m_pVersionCombo->setEnabled(true);
            int versionIndex = 0;
            for (const djinterop::semantic_version& version : el::all_versions) {
                m_pVersionCombo->insertItem(0,
                        QString::fromStdString(el::version_name(version)),
                        QVariant{versionIndex});
                if (version == el::version_latest_firmware) {
                    // Latest firmware version is the default selection.
                    m_pVersionCombo->setCurrentIndex(0);
                }

                ++versionIndex;
            }
            return;
        }

        // Find out version of the existing database, and set the displayed
        // version widget accordingly.  Changing the schema version of existing
        // databases is not currently supported.
        djinterop::database db = el::load_database(databaseDirectory.toStdString());
        const auto version = db.version();

        const auto result = std::find(el::all_versions.begin(), el::all_versions.end(), version);
        if (result == el::all_versions.end()) {
            // Unknown database version.
            m_pExistingDatabaseLabel->setText(
                    tr("A database already exists in the chosen directory, "
                       "but it is of an unsupported version. Export is not "
                       "guaranteed to succeed in this situation."));
            m_pVersionCombo->clear();
            m_pVersionCombo->setEnabled(false);
        } else {
            int versionIndex = std::distance(el::all_versions.begin(), result);
            m_pExistingDatabaseLabel->setText(
                    tr("A database already exists in the chosen directory. "
                       "Exported tracks will be added into this database."));
            m_pVersionCombo->clear();
            m_pVersionCombo->insertItem(
                    0, QString::fromStdString(el::version_name(version)), QVariant{versionIndex});
            m_pVersionCombo->setEnabled(false);
        }

    } catch (std::exception& e) {
        m_pExistingDatabaseLabel->setText(
                tr("A database already exists in the chosen directory, "
                   "but there was a problem loading it. Export is not "
                   "guaranteed to succeed in this situation."));
        m_pVersionCombo->clear();
        m_pVersionCombo->setEnabled(false);
    }
}

} // namespace mixxx
