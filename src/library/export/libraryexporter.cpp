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
        connect(m_pDialog.get(), &DlgLibraryExport::startEnginePrimeExport, this, &LibraryExporter::beginEnginePrimeExport);
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

    // Construct a dialog to monitor job progress and offer cancellation.
    auto pd = make_parented<QProgressDialog>(this);
    pd->setLabelText(tr("Exporting to Engine Prime..."));
    pd->setMinimumDuration(0);
    connect(pJobThread, &EnginePrimeExportJob::jobMaximum, pd, &QProgressDialog::setMaximum);
    connect(pJobThread, &EnginePrimeExportJob::jobProgress, pd, &QProgressDialog::setValue);
    connect(pJobThread, &EnginePrimeExportJob::finished, pd, &QObject::deleteLater);
    connect(pd, &QProgressDialog::canceled, pJobThread, &EnginePrimeExportJob::cancel);

    pJobThread->start();
}

} // namespace mixxx
