#ifdef __DJINTEROP__
#pragma once

#include <memory>

#include <QWidget>

#include "library/export/dlglibraryexport.h"
#include "library/export/enginelibraryexportrequest.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"

class TrackCollectionManager;

namespace mixxx {
class TrackLoader;

/// The LibraryExporter class allows an export of the Mixxx library to be
/// initiated.  It can present a dialog that gathers information from the user
/// about the nature of the export, and schedules a job to perform the export.
/// The class uses libdjinterop to perform the export.
class LibraryExporter : public QWidget {
    Q_OBJECT
  public:
    LibraryExporter(QWidget* parent,
            UserSettingsPointer pConfig,
            TrackCollectionManager& trackCollectionManager);

  public slots:
    /// Begin the process of a library export.
    void requestExport();

  private slots:
    void beginEngineLibraryExport(EngineLibraryExportRequest request);

  private:
    UserSettingsPointer m_pConfig;
    TrackCollectionManager& m_trackCollectionManager;
    TrackLoader* m_pTrackLoader;
    parented_ptr<DlgLibraryExport> m_pDialog;
};

} // namespace mixxx
#endif
