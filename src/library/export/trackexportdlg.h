#ifndef DLGTRACKEXPORT_H
#define DLGTRACKEXPORT_H

#include <QDialog>
#include <QScopedPointer>
#include <QString>
#include <future>

#include "library/export/trackexportworker.h"
#include "library/export/ui_dlgtrackexport.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"

// A dialog for interacting with the track exporter in an interactive manner.
// Handles errors and user interactions.
class TrackExportDlg : public QDialog, public Ui::DlgTrackExport {
    Q_OBJECT
  public:
    enum class OverwriteMode {
        ASK,
        OVERWRITE_ALL,
        SKIP_ALL,
    };

    // The dialog is prepared, but not shown on construction.  Does not
    // take ownership of the export worker.
    TrackExportDlg(QWidget *parent, UserSettingsPointer pConfig,
                   TrackExportWorker* worker);
    virtual ~TrackExportDlg() { }

  public slots:
    void slotProgress(QString filename, int progress, int count);
    void slotAskOverwriteMode(
            QString filename,
            std::promise<TrackExportWorker::OverwriteAnswer>* promise);
    void cancelButtonClicked();

  protected:
    // First pops up a directory selector on show(), then does the actual
    // copying.
    void showEvent(QShowEvent* event) override;

  private:
    // Called when progress is complete or the procedure has been canceled.
    // Displays a final message box indicating success or failure.
    // Makes sure the exporter thread has exited.
    void finish();

    UserSettingsPointer m_pConfig;
    TrackPointerList m_tracks;
    TrackExportWorker* m_worker;
};

#endif  // DLGTRACKEXPORT_H
