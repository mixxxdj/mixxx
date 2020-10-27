#include "library/export/libraryexporter.h"

#include <QProgressDialog>
#include <QThreadPool>

#include "library/export/engineprimeexportjob.h"
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

    m_pDialog->setSelectedCrate(initialSelectedCrate);
}

void LibraryExporter::beginEnginePrimeExport(
        EnginePrimeExportRequest request) {
    // Note that the job will run in a background thread.
    auto pJobThread = make_parented<EnginePrimeExportJob>(
            this,
            m_pTrackCollectionManager,
            std::move(request));
    connect(pJobThread, &EnginePrimeExportJob::finished, pJobThread, &QObject::deleteLater);
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
            [](QString message) {
                QMessageBox::critical(nullptr,
                        tr("Export Failed"),
                        QString{tr("Export failed: %1")}.arg(message));
            });

    // Construct a dialog to monitor job progress and offer cancellation.
    auto pProgressDlg = make_parented<QProgressDialog>(this);
    pProgressDlg->setLabelText(tr("Exporting to Engine Prime..."));
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
