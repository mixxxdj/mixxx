#ifdef __DJINTEROP__
#pragma once

#include <memory>

#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QTreeWidget>
#include <QWidget>

#include "library/export/enginelibraryexportrequest.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"

class TrackCollectionManager;

namespace mixxx {

/// The DlgLibraryExport class is a UI window that gathers information from
/// the user about how they would like to export the Mixxx library.
///
/// Currently, the dialog only supports exporting to the Engine Library format,
/// but in future it is expected that this dialog could be expanded to include
/// other formats, and generate different export signals accordingly.
class DlgLibraryExport : public QDialog {
    Q_OBJECT

  public:
    DlgLibraryExport(
            QWidget* parent, UserSettingsPointer pConfig, TrackCollectionManager& trackCollectionManager);

  signals:
    /// The startExport signal is emitted when sufficient information has been
    /// gathered from the user to kick off an Engine Library export, and details
    /// of the request are provided as part of the signal.
    void startEngineLibraryExport(EngineLibraryExportRequest) const;

  private slots:
    void exportWholeLibrarySelected();
    void exportSelectedCratedSelected();
    void browseExportDirectory();
    void exportRequested();

  private:
    UserSettingsPointer m_pConfig;
    TrackCollectionManager& m_trackCollectionManager;

    parented_ptr<QListWidget> m_pCratesList;
    parented_ptr<QLineEdit> m_pBaseDirectoryTextField;
    parented_ptr<QLineEdit> m_pDatabaseDirectoryTextField;
    parented_ptr<QLineEdit> m_pMusicDirectoryTextField;
};

} // namespace mixxx
#endif
