#include "library/export/dlglibraryexport.h"

#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStandardPaths>
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

void populateCrates(
        QListWidget& listWidget,
        const TrackCollection& trackCollection) {
    // Populate list of crates.
    CrateSelectResult crates = trackCollection.crates().selectCrates();
    Crate crate;
    while (crates.populateNext(&crate)) {
        auto pItem = std::make_unique<QListWidgetItem>(crate.getName());
        pItem->setData(Qt::UserRole, crate.getId().value());
        listWidget.addItem(pItem.release());
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
    populateCrates(*m_pCratesList, *m_pTrackCollectionManager->internalCollection());

    // Read-only text fields showing key directories for export.
    m_pBaseDirectoryTextField = make_parented<QLineEdit>();
    m_pBaseDirectoryTextField->setReadOnly(true);
    m_pDatabaseDirectoryTextField = make_parented<QLineEdit>();
    m_pDatabaseDirectoryTextField->setReadOnly(true);
    m_pMusicDirectoryTextField = make_parented<QLineEdit>();
    m_pMusicDirectoryTextField->setReadOnly(true);

    // Drop-down for choosing exported database version.
    m_pVersionCombo = make_parented<QComboBox>();
    int versionIndex = 0;
    for (const djinterop::semantic_version& version : el::all_versions) {
        std::string label = el::version_name(version);
        m_pVersionCombo->insertItem(
                0, QString::fromStdString(label), QVariant{versionIndex});
        if (version == el::version_latest_firmware) {
            // Latest firmware version is the default selection.
            m_pVersionCombo->setCurrentIndex(0);
        }

        ++versionIndex;
    }

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
    pExportDirLayout->addWidget(m_pBaseDirectoryTextField);
    pExportDirLayout->addWidget(pExportDirBrowseButton);

    auto pFormLayout = make_parented<QFormLayout>();
    pFormLayout->addRow(tr("Base export directory"), pExportDirLayout);
    pFormLayout->addRow(tr("Database version"), m_pVersionCombo);
    pFormLayout->addRow(tr("Engine Prime database export directory"),
            m_pDatabaseDirectoryTextField);
    pFormLayout->addRow(tr("Copy music files to"), m_pMusicDirectoryTextField);

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
        auto currCrateId = pItem->data(Qt::UserRole).toInt();
        if (currCrateId == crateId->value()) {
            m_pCratesList->setCurrentItem(pItem);
            return;
        }
    }
}

void DlgLibraryExport::browseExportDirectory() {
    QString lastExportDirectory =
            m_pConfig->getValue(ConfigKey("[Library]", "LastLibraryExportDirectory"),
                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    auto baseDirectory = QFileDialog::getExistingDirectory(
            nullptr, tr("Export Library To"), lastExportDirectory);
    if (baseDirectory.isEmpty()) {
        return;
    }
    m_pConfig->set(
            ConfigKey("[Library]", "LastLibraryExportDirectory"), ConfigValue(baseDirectory));

    QDir baseExportDirectory{baseDirectory};
    auto databaseDirectory = baseExportDirectory.filePath(
            el::default_database_dir_name);
    auto musicDirectory = baseExportDirectory.filePath(kDefaultMixxxExportDirName);

    m_pBaseDirectoryTextField->setText(baseDirectory);
    m_pDatabaseDirectoryTextField->setText(databaseDirectory);
    m_pMusicDirectoryTextField->setText(musicDirectory);
}

void DlgLibraryExport::exportRequested() {
    // Check a base export directory has been chosen
    if (m_pBaseDirectoryTextField->text() == "") {
        QMessageBox::information(this,
                tr("No Export Directory Chosen"),
                tr("No export directory was chosen. Please choose a directory "
                   "in order to export the music library."),
                QMessageBox::Ok,
                QMessageBox::Ok);
        return;
    }

    // See if an EL DB exists in the chosen dir already, and ask the user for
    // confirmation before proceeding if so.
    if (el::database_exists(m_pDatabaseDirectoryTextField->text().toStdString())) {
        int ret = QMessageBox::question(
                this,
                tr("Merge Into Existing Library?"),
                tr("There is already an existing library in directory ") +
                        m_pDatabaseDirectoryTextField->text() +
                        tr("\nIf you proceed, the Mixxx library will be merged into "
                           "this existing library.  Do you want to merge into the "
                           "the existing library?"),
                QMessageBox::Yes | QMessageBox::Cancel,
                QMessageBox::Cancel);
        if (ret != QMessageBox::Yes) {
            return;
        }
    }

    // Work out what version was requested.
    int versionIndex = m_pVersionCombo->currentData().toInt();
    djinterop::semantic_version exportVersion = el::all_versions[versionIndex];

    // Construct a request to export the library/crates.
    // Assumed to always be an Engine Prime export in this iteration of the
    // dialog.
    EnginePrimeExportRequest request;
    request.engineLibraryDbDir = QDir{m_pDatabaseDirectoryTextField->text()};
    request.musicFilesDir = QDir{m_pMusicDirectoryTextField->text()};
    request.exportVersion = exportVersion;
    request.exportSelectedCrates = m_pCratesList->isEnabled();
    if (request.exportSelectedCrates) {
        for (auto* pItem : m_pCratesList->selectedItems()) {
            CrateId id{pItem->data(Qt::UserRole).value<int>()};
            request.crateIdsToExport.insert(id);
        }
    }

    emit startEnginePrimeExport(std::move(request));
    accept();
}

} // namespace mixxx
