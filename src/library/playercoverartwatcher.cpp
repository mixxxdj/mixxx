#include "library/playercoverartwatcher.h"

#include <QPixmap>

#include "library/coverart.h"
#include "library/coverartcache.h"
#include "mixer/basetrackplayer.h"
#include "moc_playercoverartwatcher.cpp"
#include "track/track.h"

namespace mixxx {

PlayerCoverArtWatcher::PlayerCoverArtWatcher(BaseTrackPlayer* pPlayer, QObject* pParent)
        : QObject(pParent) {
    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache) {
        connect(pCache,
                &CoverArtCache::coverFound,
                this,
                &PlayerCoverArtWatcher::slotCoverFound);
    }
    if (pPlayer) {
        connect(pPlayer,
                &BaseTrackPlayer::newTrackLoaded,
                this,
                &PlayerCoverArtWatcher::slotLoadTrack);
        connect(pPlayer,
                &BaseTrackPlayer::loadingTrack,
                this,
                &PlayerCoverArtWatcher::slotLoadingTrack);
    }
}

void PlayerCoverArtWatcher::requestCover() {
    if (m_pLoadedTrack) {
        CoverArtCache::requestTrackCover(this, m_pLoadedTrack);
    }
}

// static
bool PlayerCoverArtWatcher::isCoverOfTrack(
        const CoverInfo& foundCover, const CoverInfo& trackCover) {
    if (foundCover.trackLocation == trackCover.trackLocation) {
        return true;
    }
    return foundCover.hasCacheKey() &&
            trackCover.hasCacheKey() &&
            foundCover.cacheKey() == trackCover.cacheKey();
}

void PlayerCoverArtWatcher::slotLoadTrack(TrackPointer pTrack) {
    disconnectLoadedTrack();
    m_pLoadedTrack = pTrack;
    emit trackChanged();
    if (m_pLoadedTrack) {
        connect(m_pLoadedTrack.get(),
                &Track::coverArtUpdated,
                this,
                &PlayerCoverArtWatcher::coverOutdated);
        emit coverOutdated();
    }
}

void PlayerCoverArtWatcher::slotLoadingTrack(TrackPointer, TrackPointer pOldTrack) {
    if (pOldTrack == m_pLoadedTrack) {
        disconnectLoadedTrack();
    }
    m_pLoadedTrack.reset();
    emit trackChanged();
}

void PlayerCoverArtWatcher::slotCoverFound(
        const QObject* pRequester,
        const CoverInfo& coverInfo,
        const QPixmap& pixmap) {
    if (pRequester != this || !m_pLoadedTrack) {
        return;
    }
    if (!isCoverOfTrack(coverInfo, m_pLoadedTrack->getCoverInfoWithLocation())) {
        return;
    }
    emit coverFound(pixmap);
}

void PlayerCoverArtWatcher::disconnectLoadedTrack() {
    if (m_pLoadedTrack) {
        disconnect(m_pLoadedTrack.get(),
                &Track::coverArtUpdated,
                this,
                &PlayerCoverArtWatcher::coverOutdated);
    }
}

} // namespace mixxx
