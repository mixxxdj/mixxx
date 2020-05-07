#ifdef __DJINTEROP__
#include "library/export/libraryexporter.h"

#include <QProgressDialog>
#include <QThreadPool>

#include "library/export/enginelibraryexportjob.h"
#include "library/trackloader.h"

namespace mixxx {

LibraryExporter::LibraryExporter(QWidget* parent,
        UserSettingsPointer pConfig,
        TrackCollectionManager& trackCollectionManager)
        : QWidget{parent},
          m_pConfig{std::move(pConfig)},
          m_trackCollectionManager{trackCollectionManager},
          m_pTrackLoader{nullptr} {
    m_pTrackLoader = new TrackLoader(&m_trackCollectionManager, this);
}

void LibraryExporter::requestExport() {
    if (!m_pDialog) {
        m_pDialog = make_parented<DlgLibraryExport>(this, m_pConfig, m_trackCollectionManager);
        connect(m_pDialog.get(),
                SIGNAL(startEngineLibraryExport(EngineLibraryExportRequest)),
                this,
                SLOT(beginEngineLibraryExport(EngineLibraryExportRequest)));
    } else {
        m_pDialog->show();
        m_pDialog->raise();
        m_pDialog->setWindowState(
                (m_pDialog->windowState() & ~Qt::WindowMinimized) |
                Qt::WindowActive);
    }
}

void LibraryExporter::beginEngineLibraryExport(
        EngineLibraryExportRequest request) {
    // Note that the job will run in a background thread.
    auto* pJobThread = new EngineLibraryExportJob{
            this, m_trackCollectionManager, *m_pTrackLoader, std::move(request)};
    connect(pJobThread, &EngineLibraryExportJob::finished, pJobThread, &QObject::deleteLater);

    // Construct a modal dialog to monitor job progress.
    // TODO(mr-smidge) - dialog doesn't appear to update with new progress until after track export?
    auto *pd = new QProgressDialog(this);
    pd->setLabelText(tr("Exporting to Engine Library..."));
    pd->setMinimumDuration(0);
    connect(pJobThread, &EngineLibraryExportJob::jobMaximum, pd, &QProgressDialog::setMaximum);
    connect(pJobThread, &EngineLibraryExportJob::jobProgress, pd, &QProgressDialog::setValue);
    connect(pJobThread, &EngineLibraryExportJob::finished, pd, &QObject::deleteLater);
    connect(pd, &QProgressDialog::canceled, pJobThread, &EngineLibraryExportJob::cancel);

    pJobThread->start();
}

} // namespace mixxx
#endif
