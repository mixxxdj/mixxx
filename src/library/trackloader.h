#pragma once

#include <QObject>
#include <QPointer>

#include "track/track.h"
#include "track/trackref.h"

class TrackCollectionManager;

namespace mixxx {

// Loads tracks thread-safe and asynchronously from the local collection.
// Must be located in the same thread where TrackCollectionManager lives!
class TrackLoader: public QObject {
    Q_OBJECT

  public:
    explicit TrackLoader(
            TrackCollectionManager* trackCollectionManager,
            QObject* parent = nullptr);
    ~TrackLoader() override = default;

    // Thread-safe, possibly asynchronous invocation. May emit
    // the trackLoaded signal directly when invoked from the
    // same thread!
    void invokeLoadTrack(
            TrackRef trackRef,
            Qt::ConnectionType connectionType = Qt::AutoConnection);

  public slots:
    void loadTrack(
            TrackRef trackRef);

  signals:
    // A nullptr indicates failure. Receivers need to
    // filter the tracks they have actually requested
    // if this object is used by different requesters.
    void trackLoaded(
            TrackRef trackRef,
            TrackPointer trackPtr);

  private:
    const QPointer<TrackCollectionManager> m_trackCollectionManager;
};

} // namespace mixxx
