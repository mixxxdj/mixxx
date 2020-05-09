#ifdef __DJINTEROP__
#include "library/export/dlglibraryexport.h"

#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QStandardPaths>

#include <djinterop/enginelibrary.hpp>

#include "library/crate/crateid.h"
#include "library/export/engineprimeexportrequest.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"

static const QString DefaultMixxxExportDirName = "mixxx-export";

namespace el = djinterop::enginelibrary;

namespace mixxx {

DlgLibraryExport::DlgLibraryExport(
        QWidget* parent, UserSettingsPointer pConfig, TrackCollectionManager& trackCollectionManager)
        : QDialog(parent), m_pConfig{pConfig}, m_trackCollectionManager{trackCollectionManager} {
    // Selectable list of crates from the Mixxx library.
    m_pCratesList = make_parented<QListWidget>();
    m_pCratesList->setSelectionMode(QListWidget::ExtendedSelection);
    {
        // Populate list of crates.
        auto crates = m_trackCollectionManager.internalCollection()->crates().selectCrates();
        Crate crate;
        while (crates.populateNext(&crate)) {
            auto pItem = std::make_unique<QListWidgetItem>(crate.getName());
            pItem->setData(Qt::UserRole, crate.getId().value());
            m_pCratesList->addItem(pItem.release());
        }
    }

    // Read-only text fields showing key directories for export.
    m_pBaseDirectoryTextField = make_parented<QLineEdit>();
    m_pBaseDirectoryTextField->setReadOnly(true);
    m_pDatabaseDirectoryTextField = make_parented<QLineEdit>();
    m_pDatabaseDirectoryTextField->setReadOnly(true);
    m_pMusicDirectoryTextField = make_parented<QLineEdit>();
    m_pMusicDirectoryTextField->setReadOnly(true);

    // Radio buttons to allow choice between exporting the whole music library
    // or just tracks in a selection of crates.
    auto pWholeLibraryRadio = make_parented<QRadioButton>(tr("Entire music library"));
    pWholeLibraryRadio->setChecked(true);
    this->exportWholeLibrarySelected();
    connect(pWholeLibraryRadio,
            &QRadioButton::clicked,
            this,
            &DlgLibraryExport::exportWholeLibrarySelected);
    auto pCratesRadio = make_parented<QRadioButton>(tr("Selected crates"));
    connect(pCratesRadio,
            &QRadioButton::clicked,
            this,
            &DlgLibraryExport::exportSelectedCratedSelected);

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
    pFormLayout->addRow(tr("Engine Prime database export directory"), m_pDatabaseDirectoryTextField);
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
    pLayout->addWidget(pWholeLibraryRadio, 0, 0);
    pLayout->addWidget(pCratesRadio, 1, 0);
    pLayout->addWidget(m_pCratesList, 2, 0);
    pLayout->addLayout(pFormLayout, 0, 1, 3, 1);
    pLayout->addLayout(pButtonBarLayout, 3, 0, 1, 2);

    setLayout(pLayout);
    setWindowTitle(tr("Export Library"));

    show();
    raise();
    activateWindow();
}

void DlgLibraryExport::exportWholeLibrarySelected() {
    // Disallow selection of individual crates.
    m_pCratesList->setEnabled(false);
}

void DlgLibraryExport::exportSelectedCratedSelected() {
    // Allow selection of individual crates
    m_pCratesList->setEnabled(true);
}

void DlgLibraryExport::browseExportDirectory() {
    QString lastExportDirectory =
            m_pConfig->getValue(ConfigKey("[Library]", "LastLibraryExportDirectory"),
                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    auto baseDirectory =
            QFileDialog::getExistingDirectory(NULL, tr("Export Library To"), lastExportDirectory);
    if (baseDirectory.isEmpty()) {
        return;
    }
    m_pConfig->set(
            ConfigKey("[Library]", "LastLibraryExportDirectory"), ConfigValue(baseDirectory));

    QDir baseExportDirectory{baseDirectory};
    auto databaseDirectory = baseExportDirectory.filePath(
            el::default_database_dir_name);
    auto musicDirectory = baseExportDirectory.filePath(DefaultMixxxExportDirName);

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

    // Construct a request to export the library/crates.
    // Assumed to always be an Engine Prime export in this iteration of the
    // dialog.
    EnginePrimeExportRequest request;
    request.engineLibraryDbDir = QDir{m_pDatabaseDirectoryTextField->text()};
    request.musicFilesDir = QDir{m_pMusicDirectoryTextField->text()};
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
#endif
