#pragma once

#include <QObject>
#include <QPointer>

#include "track/track_decl.h"
#include "track/trackref.h"

class TrackCollectionManager;

namespace mixxx {

// Loads tracks thread-safe and asynchronously from the local collection.
// Is moved into the thread of TrackCollectionManager during construction
// and must remain there!
class TrackLoader: public QObject {
    Q_OBJECT

  public:
    explicit TrackLoader(
            TrackCollectionManager* trackCollectionManager,
            QObject* parent = nullptr);
    ~TrackLoader() override = default;

    // Explicit, thread-safe invocation of the corresponding slot.
    void invokeSlotLoadTrack(
            TrackRef trackRef,
            Qt::ConnectionType connectionType = Qt::AutoConnection);

  public slots:
    // Asynchronously try to load the referenced track from the
    // internal track collection. If the track is not already
    // contained in the database it will implicitly be added to
    // the track collection from the file location if available.
    // The result of this operation is propagated by the corresponding
    // signal.
    void slotLoadTrack(
            TrackRef trackRef);

  signals:
    // A nullptr indicates failure to load the track. Receivers need to
    // filter the loaded tracks they have actually requested if the
    // corresponding slot is invoked by multiple signal senders or
    // from different clients!
    void trackLoaded(
            TrackRef trackRef,
            TrackPointer trackPtr);

  private:
    const QPointer<TrackCollectionManager> m_trackCollectionManager;
};

} // namespace mixxx
