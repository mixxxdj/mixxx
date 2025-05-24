#include "javascriptplayerproxy.h"

#include "moc_javascriptplayerproxy.cpp"

JavascriptPlayerProxy::JavascriptPlayerProxy(BaseTrackPlayer* pTrackPlayer, QObject* parent)
        : QObject(parent),
          m_pTrackPlayer(pTrackPlayer) {
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
            &JavascriptPlayerProxy::trackUnloaded);
    connect(this,
            &JavascriptPlayerProxy::trackChanged,
            this,
            &JavascriptPlayerProxy::slotTrackChanged);
    if (m_pTrackPlayer && m_pTrackPlayer->getLoadedTrack()) {
        slotTrackLoaded(pTrackPlayer->getLoadedTrack());
    }
}

void JavascriptPlayerProxy::slotTrackLoaded(TrackPointer pTrack) {
    m_pCurrentTrack = pTrack;

    if (pTrack != nullptr) {
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
    }
    emit trackChanged();
    emit trackLoaded();
}

void JavascriptPlayerProxy::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    VERIFY_OR_DEBUG_ASSERT(pOldTrack == m_pCurrentTrack) {
        qWarning() << "Javascript Player proxy was expected to contain "
                   << pOldTrack.get() << "as active track but got"
                   << m_pCurrentTrack.get();
    }

    if (pNewTrack.get() == m_pCurrentTrack.get()) {
        emit trackLoading();
        return;
    }

    if (m_pCurrentTrack != nullptr) {
        disconnect(m_pCurrentTrack.get(), nullptr, this, nullptr);
    }
    m_pCurrentTrack = pNewTrack;
    emit trackChanged();
    emit trackLoading();
}

void JavascriptPlayerProxy::slotTrackChanged() {
    emit artistChanged();
    emit titleChanged();
    emit albumChanged();
    emit albumArtistChanged();
    emit genreChanged();
    emit composerChanged();
    emit groupingChanged();
    emit yearChanged();
    emit trackNumberChanged();
    emit trackTotalChanged();
}

bool JavascriptPlayerProxy::isLoaded() const {
    return m_pTrackPlayer->getLoadedTrack() != nullptr;
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
