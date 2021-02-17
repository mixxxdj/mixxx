#pragma once

#include <QAtomicInteger>
#include <QList>
#include <QMutex>
#include <QQueue>
#include <QSet>
#include <QSharedPointer>
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
            QSharedPointer<EnginePrimeExportRequest> pRequest);

    /// Run the export job.
    void run() override;

  signals:
    /// Informs of the maximum number that will be emitted to convey progress.
    void jobMaximum(int maximum);

    /// Informs of progress through the job, up to the pre-signalled maximum.
    void jobProgress(int progress);

    /// Inform of a completed export job.
    void completed(int numTracksExported, int numCratesExported);

    /// Inform of a failed export job.
    void failed(const QString& message);

  public slots:
    /// Request cancellation of any running export job.
    void slotCancel();

  private slots:
    // These slots are used to load data from the Mixxx database on the main
    // thread of the application, which will be different to the worker thread
    // used by an instance of this class.
    void loadIds(const QSet<CrateId>& crateIdsToExport);
    void loadTrack(const TrackRef& trackRef);
    void loadCrate(const CrateId& crateId);

  private:
    QList<TrackRef> m_trackRefs;
    QList<CrateId> m_crateIds;
    TrackPointer m_pLastLoadedTrack;
    std::unique_ptr<Waveform> m_pLastLoadedWaveform;
    Crate m_lastLoadedCrate;
    QList<TrackId> m_lastLoadedCrateTrackIds;

    QAtomicInteger<int> m_cancellationRequested;

    TrackCollectionManager* m_pTrackCollectionManager;
    QSharedPointer<EnginePrimeExportRequest> m_pRequest;

    QString m_lastErrorMessage;
};

} // namespace mixxx
