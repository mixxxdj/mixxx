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
        subclassesSlotTrackLoaded();
        slotWaveformChanged();
    }
    emit trackChanged();
    emit trackLoaded();
}

void JavascriptPlayerProxy::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    VERIFY_OR_DEBUG_ASSERT(pOldTrack == m_pCurrentTrack) {
        qWarning() << "QML Player proxy was expected to contain "
                   << pOldTrack.get() << "as active track but got"
                   << m_pCurrentTrack.get();
    }

    if (pNewTrack.get() == m_pCurrentTrack.get()) {
        emit trackLoading();
        return;
    }

    const TrackPointer pTrack = m_pCurrentTrack;
    if (pTrack != nullptr) {
        disconnect(pTrack.get(), nullptr, this, nullptr);
    }
    m_pCurrentTrack.reset();
    m_pCurrentTrack = pNewTrack;
    m_waveformTexture = QImage();
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

    emit waveformLengthChanged();
    emit waveformTextureChanged();
    emit waveformTextureSizeChanged();
    emit waveformTextureStrideChanged();

    subclassesSlotTrackChanged();
}

void JavascriptPlayerProxy::slotWaveformChanged() {
    emit waveformLengthChanged();
    emit waveformTextureSizeChanged();
    emit waveformTextureStrideChanged();

    const TrackPointer pTrack = m_pCurrentTrack;
    if (!pTrack) {
        return;
    }
    const ConstWaveformPointer pWaveform =
            pTrack->getWaveform();
    if (!pWaveform) {
        return;
    }
    const int textureWidth = pWaveform->getTextureStride();
    const int textureHeight = pWaveform->getTextureSize() / pWaveform->getTextureStride();

    const WaveformData* data = pWaveform->data();
    // Make a copy of the waveform data, stripping the stems portion. Note that the datasize is
    // different from the texture size -- we want the full texture size so the upload works. See
    // m_data in waveform/waveform.h.
    m_waveformData.resize(pWaveform->getTextureSize());
    for (int i = 0; i < pWaveform->getDataSize(); i++) {
        m_waveformData[i] = data[i].filtered;
    }

    m_waveformTexture =
            QImage(reinterpret_cast<const uchar*>(m_waveformData.data()),
                    textureWidth,
                    textureHeight,
                    QImage::Format_RGBA8888);
    DEBUG_ASSERT(!m_waveformTexture.isNull());
    emit waveformTextureChanged();
}

int JavascriptPlayerProxy::getWaveformLength() const {
    const TrackPointer pTrack = m_pCurrentTrack;
    if (pTrack) {
        const ConstWaveformPointer pWaveform = pTrack->getWaveform();
        if (pWaveform) {
            return pWaveform->getDataSize();
        }
    }
    return 0;
}

QString JavascriptPlayerProxy::getWaveformTexture() const {
    if (m_waveformTexture.isNull()) {
        return QString();
    }
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    m_waveformTexture.save(&buffer, "png");

    QString imageData = QString::fromLatin1(byteArray.toBase64().data());
    if (imageData.isEmpty()) {
        return QString();
    }

    return QStringLiteral("data:image/png;base64,") + imageData;
}

int JavascriptPlayerProxy::getWaveformTextureSize() const {
    const TrackPointer pTrack = m_pCurrentTrack;
    if (pTrack) {
        const ConstWaveformPointer pWaveform = pTrack->getWaveform();
        if (pWaveform) {
            return pWaveform->getTextureSize();
        }
    }
    return 0;
}

int JavascriptPlayerProxy::getWaveformTextureStride() const {
    const TrackPointer pTrack = m_pCurrentTrack;
    if (pTrack) {
        const ConstWaveformPointer pWaveform = pTrack->getWaveform();
        if (pWaveform) {
            return pWaveform->getTextureStride();
        }
    }
    return 0;
}

bool JavascriptPlayerProxy::isLoaded() const {
    return m_pCurrentTrack != nullptr;
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
