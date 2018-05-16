#include "metadatabroadcast.h"

#include "mixer/playerinfo.h"
#include "moc_metadatabroadcast.cpp"
#include "track/track.h"

MetadataBroadcast::MetadataBroadcast() {
    /*
    connect(&PlayerInfo::instance(),
            SIGNAL(trackLoaded(QString, TrackPointer)),
            this,
            SLOT(slotTrackLoaded(QString, TrackPointer)));
    connect(&PlayerInfo::instance(),
            SIGNAL(trackUnloaded(QString, TrackPointer)),
            this,
            SLOT(slotTrackUnloaded(QString, TrackPointer)));
    connect(&PlayerInfo::instance(),
            SIGNAL(trackPaused(QString, TrackPointer)),
            this,
            SLOT(slotTrackPaused(QString, TrackPointer)));
    connect(&PlayerInfo::instance(),
            SIGNAL(trackResumed(QString, TrackPointer)),
            this,
            SLOT(slotTrackResumed(QString, TrackPointer)));
*/
}

void MetadataBroadcast::slotTrackLoaded(QString group, TrackPointer pTrack) {
    if (!pTrack)
        return;
    qDebug() << "Track " << pTrack->getTitle() << "loaded in group " << group << ".";
}

void MetadataBroadcast::slotTrackUnloaded(QString group, TrackPointer pTrack) {
    if (!pTrack)
        return;
    qDebug() << "Track " << pTrack->getTitle() << "unloaded in group " << group << ".";
}

void MetadataBroadcast::slotTrackPaused(QString group, TrackPointer pTrack) {
    if (!pTrack)
        return;
    qDebug() << "Track " << pTrack->getTitle() << "paused in group " << group << ".";
}

void MetadataBroadcast::slotTrackResumed(QString group, TrackPointer pTrack) {
    if (!pTrack)
        return;
    qDebug() << "Track " << pTrack->getTitle() << "resumed in group " << group << ".";
}
