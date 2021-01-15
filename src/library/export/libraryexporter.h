#pragma once

#include <QSharedPointer>
#include <QWidget>
#include <memory>

#include "library/export/dlglibraryexport.h"
#include "library/export/engineprimeexportrequest.h"
#include "library/trackset/crate/crateid.h"
#include "preferences/usersettings.h"
#include "util/optional.h"
#include "util/parented_ptr.h"

class TrackCollectionManager;

namespace mixxx {

/// The LibraryExporter class allows an export of the Mixxx library to be
/// initiated.  It can present a dialog that gathers information from the user
/// about the nature of the export, and schedules a job to perform the export.
class LibraryExporter : public QWidget {
    Q_OBJECT
  public:
    LibraryExporter(QWidget* parent,
            UserSettingsPointer pConfig,
            TrackCollectionManager* pTrackCollectionManager);

  public slots:
    /// Begin the process of a library export.
    void slotRequestExport() {
        requestExportWithOptionalInitialCrate(std::nullopt);
    }

    /// Begin the process of a library export, with an initial crate set.
    void slotRequestExportWithInitialCrate(CrateId initialSelectedCrate) {
        requestExportWithOptionalInitialCrate(
                std::make_optional(initialSelectedCrate));
    }

  private slots:
    void beginEnginePrimeExport(QSharedPointer<mixxx::EnginePrimeExportRequest> pRequest);

  private:
    void requestExportWithOptionalInitialCrate(
            std::optional<CrateId> initialSelectedCrate);

    UserSettingsPointer m_pConfig;
    TrackCollectionManager* m_pTrackCollectionManager;
    parented_ptr<DlgLibraryExport> m_pDialog;
};

} // namespace mixxx
