#ifndef DLGTRACKEXPORT_H
#define DLGTRACKEXPORT_H

#include <future>

#include <QDialog>
#include <QString>
#include <QScopedPointer>
#include <QTime>

#include "configobject.h"
#include "library/export/trackexport.h"
#include "library/export/ui_dlgtrackexport.h"
#include "trackinfoobject.h"

// A dialog for interacting with the track exporter in an interactive manner.
// Handles errors and user interactions.
class DlgTrackExport: public QDialog, public Ui::DlgTrackExport {
    Q_OBJECT
  public:
    enum class OverwriteMode {
        ASK,
        OVERWRITE_ALL,
        SKIP_ALL,
    };

    // The dialog is prepared, but not shown on construction.
    DlgTrackExport(QWidget *parent, ConfigObject<ConfigValue>* pConfig,
                   QList<TrackPointer> tracks);
    virtual ~DlgTrackExport() { }

    // Displays a folder selection box to select the destination
    // folder.  If the user cancels the folder selection, returns false and
    // the dialog should not be shown.
    // MUST be called before .exec()ing the dialog.
    bool selectDestinationDirectory();

  public slots:
    void slotProgress(QString filename, int progress, int count);
    void slotAskOverwriteMode(
            QString filename,
            std::promise<TrackExport::OverwriteAnswer>* promise);
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

    ConfigObject<ConfigValue>* m_pConfig;
    QList<TrackPointer> m_tracks;
    QScopedPointer<TrackExport> m_exporter;
};

#endif  // DLGTRACKEXPORT_H
