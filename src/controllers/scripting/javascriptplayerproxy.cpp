#include "javascriptplayerproxy.h"

#include "moc_javascriptplayerproxy.cpp"

JavascriptPlayerProxy::JavascriptPlayerProxy(BaseTrackPlayer* pTrackPlayer, QObject* parent)
        : QObject(parent),
          m_pTrackPlayer(pTrackPlayer) {
    if (m_pTrackPlayer && m_pTrackPlayer->getLoadedTrack()) {
        slotTrackLoaded(pTrackPlayer->getLoadedTrack());
    }

    connect(m_pTrackPlayer,
            &BaseTrackPlayer::loadingTrack,
            this,
            &JavascriptPlayerProxy::slotLoadingTrack);
    connect(m_pTrackPlayer,
            &BaseTrackPlayer::newTrackLoaded,
            this,
            &JavascriptPlayerProxy::slotTrackLoaded);
    connect(m_pTrackPlayer,
            &BaseTrackPlayer::playerEmpty,
            this,
            [this]() {
                disconnectTrack();
                emit trackUnloaded();
            });
}

void JavascriptPlayerProxy::slotTrackLoaded(TrackPointer pTrack) {
    m_pCurrentTrack = pTrack;
    if (pTrack == nullptr) {
        emit trackUnloaded();
        return;
    }

    connect(pTrack.get(),
            &Track::artistChanged,
            this,
            &JavascriptPlayerProxy::artistChanged);
    connect(pTrack.get(),
            &Track::titleChanged,
            this,
            &JavascriptPlayerProxy::titleChanged);
    connect(pTrack.get(),
            &Track::albumChanged,
            this,
            &JavascriptPlayerProxy::albumChanged);
    connect(pTrack.get(),
            &Track::albumArtistChanged,
            this,
            &JavascriptPlayerProxy::albumArtistChanged);
    connect(pTrack.get(),
            &Track::genreChanged,
            this,
            &JavascriptPlayerProxy::genreChanged);
    connect(pTrack.get(),
            &Track::composerChanged,
            this,
            &JavascriptPlayerProxy::composerChanged);
    connect(pTrack.get(),
            &Track::groupingChanged,
            this,
            &JavascriptPlayerProxy::groupingChanged);
    connect(pTrack.get(),
            &Track::yearChanged,
            this,
            &JavascriptPlayerProxy::yearChanged);
    connect(pTrack.get(),
            &Track::trackNumberChanged,
            this,
            &JavascriptPlayerProxy::trackNumberChanged);
    connect(pTrack.get(),
            &Track::trackTotalChanged,
            this,
            &JavascriptPlayerProxy::trackTotalChanged);

    emit artistChanged(m_pCurrentTrack->getArtist());
    emit titleChanged(m_pCurrentTrack->getTitle());
    emit albumChanged(m_pCurrentTrack->getAlbum());
    emit albumArtistChanged(m_pCurrentTrack->getAlbumArtist());
    emit genreChanged(m_pCurrentTrack->getGenre());
    emit composerChanged(m_pCurrentTrack->getComposer());
    emit groupingChanged(m_pCurrentTrack->getGrouping());
    emit yearChanged(m_pCurrentTrack->getYear());
    emit trackNumberChanged(m_pCurrentTrack->getTrackNumber());
    emit trackTotalChanged(m_pCurrentTrack->getTrackTotal());
}

void JavascriptPlayerProxy::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    VERIFY_OR_DEBUG_ASSERT(pOldTrack == m_pCurrentTrack) {
        qWarning() << "Javascript Player proxy was expected to contain "
                   << pOldTrack.get() << "as active track but got"
                   << m_pCurrentTrack.get();
    }

    if (pNewTrack == m_pCurrentTrack) {
        return;
    }

    disconnectTrack();
    m_pCurrentTrack = pNewTrack;
}

void JavascriptPlayerProxy::disconnectTrack() {
    if (m_pCurrentTrack != nullptr) {
        m_pCurrentTrack->disconnect(this);
    }
}

PROPERTY_IMPL_GETTER(JavascriptPlayerProxy, QString, artist, getArtist)
PROPERTY_IMPL_GETTER(JavascriptPlayerProxy, QString, title, getTitle)
PROPERTY_IMPL_GETTER(JavascriptPlayerProxy, QString, album, getAlbum)
PROPERTY_IMPL_GETTER(JavascriptPlayerProxy, QString, albumArtist, getAlbumArtist)
PROPERTY_IMPL_GETTER(JavascriptPlayerProxy, QString, genre, getGenre)
PROPERTY_IMPL_GETTER(JavascriptPlayerProxy, QString, composer, getComposer)
PROPERTY_IMPL_GETTER(JavascriptPlayerProxy, QString, grouping, getGrouping)
PROPERTY_IMPL_GETTER(JavascriptPlayerProxy, QString, year, getYear)
PROPERTY_IMPL_GETTER(JavascriptPlayerProxy, QString, trackNumber, getTrackNumber)
PROPERTY_IMPL_GETTER(JavascriptPlayerProxy, QString, trackTotal, getTrackTotal)
