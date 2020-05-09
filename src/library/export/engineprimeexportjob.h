#ifdef __DJINTEROP__
#pragma once

#include <memory>

#include <QHash>
#include <QList>
#include <QMutex>
#include <QSet>
#include <QThread>
#include <QWaitCondition>

#include <djinterop/database.hpp>

#include "library/export/engineprimeexportrequest.h"
#include "library/trackcollectionmanager.h"
#include "library/trackloader.h"

namespace mixxx {

/// The Engine Prime export job performs the work of exporting the Mixxx
/// library to an external Engine Prime (also known as "Engine Library")
/// database, using the libdjinterop library, in accordance with the export
/// request with which it is constructed.
class EnginePrimeExportJob : public QThread {
    Q_OBJECT
  public:
    EnginePrimeExportJob(
            QObject* parent, TrackCollectionManager& trackCollectionManager, TrackLoader& trackLoader, EnginePrimeExportRequest request);

    void run() override;

  signals:
    void jobMaximum(int maximum);
    void jobProgress(int progress);

  public slots:
    void cancel();

  private slots:
    void trackLoaded(TrackRef trackRef, TrackPointer trackPtr);

  private:
    QSet<TrackRef> getAllTrackRefs() const;
    QSet<TrackRef> getTracksRefsInCrates(const QSet<CrateId>& crateIds) const;

    QList<TrackRef> m_trackQueue;
    QMutex m_trackMutex;
    QWaitCondition m_waitAnyTrack;

    TrackCollectionManager& m_trackCollectionManager;
    TrackLoader& m_trackLoader;
    EnginePrimeExportRequest m_request;
    QHash<TrackId, int64_t> m_mixxxToEnginePrimeTrackIdMap;
    std::unique_ptr<djinterop::database> m_pDb;
};

} // namespace mixxx
#endif
