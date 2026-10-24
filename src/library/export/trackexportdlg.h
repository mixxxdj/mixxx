#pragma once

#include <QDialog>
#include <future>

#include "library/export/trackexportworker.h"
#include "library/export/ui_dlgtrackexport.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"

class QString;

class TrackExportDlg : public QDialog, public Ui::DlgTrackExport {
    Q_OBJECT
  public:
    enum class OverwriteMode {
        ASK,
        OVERWRITE_ALL,
        SKIP_ALL,
    };

    TrackExportDlg(QWidget* parent, UserSettingsPointer pConfig, TrackExportWorker* worker);
    virtual ~TrackExportDlg() {
    }

  public slots:
    void slotProgress(const QString& filename, int progress, int count);
    void slotAskOverwriteMode(
            const QString& filename,
            std::promise<TrackExportWorker::OverwriteAnswer>* promise);
    void cancelButtonClicked();

  protected:
    void showEvent(QShowEvent* event) override;

  private:
    // Called when progress is complete or the procedure has been canceled.
    // Displays a final message box indicating success or failure.
    // Makes sure the exporter thread has exited.
    void finish(bool completed = true);

    bool m_finished = false;
    UserSettingsPointer m_pConfig;
    TrackPointerList m_tracks;
    TrackExportWorker* m_worker;
};