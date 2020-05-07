#pragma once

#include <QAtomicInteger>
#include <QList>
#include <QMutex>
#include <QQueue>
#include <QSet>
#include <QThread>
#include <QWaitCondition>
#include <memory>

#include "library/export/engineprimeexportrequest.h"
#include "library/trackcollectionmanager.h"
#include "library/trackset/crate/crate.h"
#include "library/trackset/crate/crateid.h"
#include "track/track.h"
#include "track/trackid.h"
#include "track/trackref.h"

namespace mixxx {

/// The Engine Prime export job performs the work of exporting the Mixxx
/// library to an external Engine Prime (also known as "Engine Library")
/// database, using the libdjinterop library, in accordance with the export
/// request with which it is constructed.
class EnginePrimeExportJob : public QThread {
    Q_OBJECT
  public:
    EnginePrimeExportJob(
            QObject* parent,
            TrackCollectionManager* pTrackCollectionManager,
            EnginePrimeExportRequest request);

    void run() override;

  signals:
    void jobMaximum(int maximum);
    void jobProgress(int progress);

  public slots:
    void cancel();

  private slots:
    // These slots are used to load data from the Mixxx database on the main
    // thread of the application, which will be different to the worker thread
    // used by an instance of this class.
    void loadIds(QSet<CrateId> crateIdsToExport);
    void loadTrack(TrackRef trackRef);
    void loadCrate(CrateId crateId);

  private:
    QMutex m_mainThreadLoadMutex;
    QWaitCondition m_waitForMainThreadLoad;

    QList<TrackRef> m_trackRefs;
    QList<CrateId> m_crateIds;
    TrackPointer m_pLastLoadedTrack;
    std::unique_ptr<Waveform> m_pLastLoadedWaveform;
    Crate m_lastLoadedCrate;
    QList<TrackId> m_lastLoadedCrateTrackIds;

    QAtomicInteger<int> m_cancellationRequested;

    TrackCollectionManager* m_pTrackCollectionManager;
    EnginePrimeExportRequest m_request;
};

} // namespace mixxx
