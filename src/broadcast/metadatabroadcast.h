#include <QObject>
#include "track/track.h"

class MetadataBroadcast : public QObject {
    Q_OBJECT
public:
    MetadataBroadcast();
public slots:
    void slotTrackLoaded(QString group, TrackPointer pTrack);
    void slotTrackUnloaded(QString group, TrackPointer pTrack);
    void slotTrackPaused(QString group, TrackPointer pTrack);
    void slotTrackResumed(QString group, TrackPointer pTrack);
};