// TrackExportWizard handles exporting a list of tracks to an external directory
//
// TODO:
//   * Offer customizable file renaming
//   * Offer the option to transcode files to the codec of choice (e.g.,
//     FLAC -> AIFF for CDJ
//   * Export sidecar metadata files for import into Mixxx

#ifndef TRACKEXPORT_H
#define TRACKEXPORT_H

#include <QString>
#include <QScopedPointer>

#include "preferences/usersettings.h"
#include "library/export/trackexportdlg.h"
#include "library/export/trackexportworker.h"
#include "track/track.h"

// A controller class for creating the export worker and UI.
class TrackExportWizard : public QObject {
  Q_OBJECT
  public:
    TrackExportWizard(QWidget *parent, UserSettingsPointer pConfig,
                      QList<TrackPointer> tracks)
            : m_parent(parent), m_pConfig(pConfig), m_tracks(tracks) { }
    virtual ~TrackExportWizard() { }

    // Displays a dialog requesting destination directory, then performs
    // track export if a folder is chosen.  Handles errors gracefully.
    void exportTracks();

  private:
    bool selectDestinationDirectory();

    QWidget* m_parent;
    UserSettingsPointer m_pConfig;
    QList<TrackPointer> m_tracks;
    QScopedPointer<TrackExportDlg> m_dialog;
    QScopedPointer<TrackExportWorker> m_worker;
};

#endif  // TRACKEXPORT_H
