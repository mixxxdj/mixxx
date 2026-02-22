#include "library/export/libraryexporter.h"

#include <QProgressDialog>

#include "library/export/engineprimeexportjob.h"
#include "library/export/engineprimeexportrequest.h"
#include "moc_libraryexporter.cpp"
#include "util/parented_ptr.h"

namespace mixxx {

LibraryExporter::LibraryExporter(QWidget* parent,
        UserSettingsPointer pConfig,
        TrackCollectionManager* pTrackCollectionManager)
        : QWidget{parent},
          m_pConfig{std::move(pConfig)},
          m_pTrackCollectionManager{pTrackCollectionManager} {
}

void LibraryExporter::requestExportWithOptionalInitialCrate(
        std::optional<CrateId> initialSelectedCrate) {
    if (!m_pDialog) {
        m_pDialog = make_parented<DlgLibraryExport>(
                this, m_pConfig, m_pTrackCollectionManager);
        connect(m_pDialog.get(),
                &DlgLibraryExport::startEnginePrimeExport,
                this,
                &LibraryExporter::beginEnginePrimeExport);
    } else {
        m_pDialog->show();
        m_pDialog->raise();
        m_pDialog->setWindowState(
                (m_pDialog->windowState() & ~Qt::WindowMinimized) |
                Qt::WindowActive);
    }

    m_pDialog->refresh();
    m_pDialog->setSelectedCrate(initialSelectedCrate);
}

void LibraryExporter::beginEnginePrimeExport(
        QSharedPointer<EnginePrimeExportRequest> pRequest) {
    // Note that the job will run in a background thread.
    auto pJobThread = make_parented<EnginePrimeExportJob>(
            this,
            m_pTrackCollectionManager,
            pRequest);
    connect(pJobThread, &EnginePrimeExportJob::finished, pJobThread, &QObject::deleteLater);

    // TODO(XXX) The conclusion of the export (succeeded/failed) could be better
    //  presented as a user notification, rather than using a message box, if
    //  such functionality is added to Mixxx.
    connect(pJobThread,
            &EnginePrimeExportJob::completed,
            this,
            [](int numTracks, int numCrates) {
                QMessageBox::information(nullptr,
                        tr("Export Completed"),
                        QString{tr("Exported %1 track(s) and %2 crate(s).")}
                                .arg(numTracks)
                                .arg(numCrates));
            });
    connect(pJobThread,
            &EnginePrimeExportJob::failed,
            this,
            [](const QString& message) {
                QMessageBox::critical(nullptr, tr("Export Failed"), message);
            });

    // Construct a dialog to monitor job progress and offer cancellation.
    auto pProgressDlg = make_parented<QProgressDialog>(this);
    pProgressDlg->setLabelText(tr("Exporting to Engine DJ..."));
    pProgressDlg->setMinimumDuration(0);
    connect(pJobThread,
            &EnginePrimeExportJob::jobMaximum,
            pProgressDlg,
            &QProgressDialog::setMaximum);
    connect(pJobThread,
            &EnginePrimeExportJob::jobProgress,
            pProgressDlg,
            &QProgressDialog::setValue);
    connect(pJobThread, &EnginePrimeExportJob::finished, pProgressDlg, &QObject::deleteLater);
    connect(pProgressDlg,
            &QProgressDialog::canceled,
            pJobThread,
            &EnginePrimeExportJob::slotCancel);

    pJobThread->start();
}

} // namespace mixxx
