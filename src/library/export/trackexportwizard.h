// TrackExportWizard handles exporting a list of tracks to an external directory
//
// TODO:
//   * Dedupe the list of trackpointers
//   * Detect duplicate filenames and munge to prevent collisions.
//   * Offer the option to transcode files to the codec of choice (e.g.,
//     FLAC -> AIFF for CDJ
//   * Export sidecar metadata files for import into Mixxx

#ifndef TRACKEXPORT_H
#define TRACKEXPORT_H

#include <QString>
#include <QScopedPointer>

#include "configobject.h"
#include "library/export/trackexportdlg.h"
#include "library/export/trackexportworker.h"
#include "trackinfoobject.h"

// A controller class for creating the export worker and UI.
class TrackExportWizard : public QObject {
  Q_OBJECT
  public:
    TrackExportWizard(QWidget *parent, ConfigObject<ConfigValue>* pConfig,
                      QList<TrackPointer> tracks)
            : m_parent(parent), m_pConfig(pConfig), m_tracks(tracks) { }
    virtual ~TrackExportWizard() { }

    // Displays a dialog requesting destination directory, then performs
    // track export if a folder is chosen.  Handles errors gracefully.
    void exportTracks();

  private:
    bool selectDestinationDirectory();

    QWidget* m_parent;
    ConfigObject<ConfigValue>* m_pConfig;
    QList<TrackPointer> m_tracks;
    QScopedPointer<TrackExportDlg> m_dialog;
    QScopedPointer<TrackExportWorker> m_worker;
};

#endif  // TRACKEXPORT_H
