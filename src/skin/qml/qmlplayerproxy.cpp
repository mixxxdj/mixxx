#include "skin/qml/qmlplayerproxy.h"

#include <QBuffer>

#include "mixer/basetrackplayer.h"
#include "skin/qml/asyncimageprovider.h"

#define PROPERTY_IMPL(TYPE, NAME, GETTER, SETTER)    \
    TYPE QmlPlayerProxy::GETTER() const {            \
        const TrackPointer pTrack = m_pCurrentTrack; \
        if (pTrack == nullptr) {                     \
            return TYPE();                           \
        }                                            \
        return pTrack->GETTER();                     \
    }                                                \
                                                     \
    void QmlPlayerProxy::SETTER(const TYPE& value) { \
        const TrackPointer pTrack = m_pCurrentTrack; \
        if (pTrack != nullptr) {                     \
            pTrack->SETTER(value);                   \
        }                                            \
    }

namespace mixxx {
namespace skin {
namespace qml {

QmlPlayerProxy::QmlPlayerProxy(BaseTrackPlayer* pTrackPlayer, QObject* parent)
        : QObject(parent), m_pTrackPlayer(pTrackPlayer) {
    connect(m_pTrackPlayer,
            &BaseTrackPlayer::loadingTrack,
            this,
            &QmlPlayerProxy::slotLoadingTrack);
    connect(m_pTrackPlayer,
            &BaseTrackPlayer::newTrackLoaded,
            this,
            &QmlPlayerProxy::slotTrackLoaded);
    connect(m_pTrackPlayer,
            &BaseTrackPlayer::playerEmpty,
            this,
            &QmlPlayerProxy::trackUnloaded);
    connect(this, &QmlPlayerProxy::trackChanged, this, &QmlPlayerProxy::slotTrackChanged);
}

void QmlPlayerProxy::slotTrackLoaded(TrackPointer pTrack) {
    m_pCurrentTrack = pTrack;
    if (pTrack != nullptr) {
        connect(pTrack.get(),
                &Track::artistChanged,
                this,
                &QmlPlayerProxy::artistChanged);
        connect(pTrack.get(),
                &Track::titleChanged,
                this,
                &QmlPlayerProxy::titleChanged);
        connect(pTrack.get(),
                &Track::albumChanged,
                this,
                &QmlPlayerProxy::albumChanged);
        connect(pTrack.get(),
                &Track::albumArtistChanged,
                this,
                &QmlPlayerProxy::albumArtistChanged);
        connect(pTrack.get(),
                &Track::genreChanged,
                this,
                &QmlPlayerProxy::genreChanged);
        connect(pTrack.get(),
                &Track::composerChanged,
                this,
                &QmlPlayerProxy::composerChanged);
        connect(pTrack.get(),
                &Track::groupingChanged,
                this,
                &QmlPlayerProxy::groupingChanged);
        connect(pTrack.get(),
                &Track::yearChanged,
                this,
                &QmlPlayerProxy::yearChanged);
        connect(pTrack.get(),
                &Track::trackNumberChanged,
                this,
                &QmlPlayerProxy::trackNumberChanged);
        connect(pTrack.get(),
                &Track::trackTotalChanged,
                this,
                &QmlPlayerProxy::trackTotalChanged);
        connect(pTrack.get(),
                &Track::commentChanged,
                this,
                &QmlPlayerProxy::commentChanged);
        connect(pTrack.get(),
                &Track::keyChanged,
                this,
                &QmlPlayerProxy::keyTextChanged);
        connect(pTrack.get(),
                &Track::colorUpdated,
                this,
                &QmlPlayerProxy::colorChanged);
        connect(pTrack.get(),
                &Track::waveformUpdated,
                this,
                &QmlPlayerProxy::slotWaveformChanged);
    }
    emit trackChanged();
    emit trackLoaded();
}

void QmlPlayerProxy::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pNewTrack);
    Q_UNUSED(pOldTrack);
    const TrackPointer pTrack = m_pCurrentTrack;
    if (pTrack != nullptr) {
        disconnect(pTrack.get(), nullptr, this, nullptr);
    }
    m_pCurrentTrack.reset();
    m_waveformTexture = QImage();
    emit trackChanged();
    emit trackLoading();
}

void QmlPlayerProxy::slotTrackChanged() {
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
    emit commentChanged();
    emit keyTextChanged();
    emit colorChanged();
    emit coverArtUrlChanged();

    emit waveformLengthChanged();
    emit waveformTextureChanged();
    emit waveformTextureSizeChanged();
    emit waveformTextureStrideChanged();
}

void QmlPlayerProxy::slotWaveformChanged() {
    emit waveformLengthChanged();
    emit waveformTextureSizeChanged();
    emit waveformTextureStrideChanged();

    const TrackPointer pTrack = m_pCurrentTrack;
    if (pTrack) {
        const ConstWaveformPointer pWaveform = pTrack->getWaveform();
        const int textureWidth = pWaveform->getTextureStride();
        const int textureHeight = pWaveform->getTextureSize() / pWaveform->getTextureStride();
        if (pWaveform) {
            const uchar* data = reinterpret_cast<const uchar*>(pWaveform->data());
            m_waveformTexture = QImage(data, textureWidth, textureHeight, QImage::Format_RGBA8888);
            emit waveformTextureChanged();
        }
    }
}

int QmlPlayerProxy::getWaveformLength() const {
    const TrackPointer pTrack = m_pCurrentTrack;
    if (pTrack) {
        const ConstWaveformPointer pWaveform = pTrack->getWaveform();
        if (pWaveform) {
            return pWaveform->getDataSize();
        }
    }
    return 0;
}

QString QmlPlayerProxy::getWaveformTexture() const {
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

int QmlPlayerProxy::getWaveformTextureSize() const {
    const TrackPointer pTrack = m_pCurrentTrack;
    if (pTrack) {
        const ConstWaveformPointer pWaveform = pTrack->getWaveform();
        if (pWaveform) {
            return pWaveform->getTextureSize();
        }
    }
    return 0;
}

int QmlPlayerProxy::getWaveformTextureStride() const {
    const TrackPointer pTrack = m_pCurrentTrack;
    if (pTrack) {
        const ConstWaveformPointer pWaveform = pTrack->getWaveform();
        if (pWaveform) {
            return pWaveform->getTextureStride();
        }
    }
    return 0;
}

PROPERTY_IMPL(QString, artist, getArtist, setArtist)
PROPERTY_IMPL(QString, title, getTitle, setTitle)
PROPERTY_IMPL(QString, album, getAlbum, setAlbum)
PROPERTY_IMPL(QString, albumArtist, getAlbumArtist, setAlbumArtist)
PROPERTY_IMPL(QString, genre, getGenre, setGenre)
PROPERTY_IMPL(QString, composer, getComposer, setComposer)
PROPERTY_IMPL(QString, grouping, getGrouping, setGrouping)
PROPERTY_IMPL(QString, year, getYear, setYear)
PROPERTY_IMPL(QString, trackNumber, getTrackNumber, setTrackNumber)
PROPERTY_IMPL(QString, trackTotal, getTrackTotal, setTrackTotal)
PROPERTY_IMPL(QString, comment, getComment, setComment)
PROPERTY_IMPL(QString, keyText, getKeyText, setKeyText)

QColor QmlPlayerProxy::getColor() const {
    const TrackPointer pTrack = m_pCurrentTrack;
    if (pTrack == nullptr) {
        return QColor();
    }
    return RgbColor::toQColor(pTrack->getColor());
}

void QmlPlayerProxy::setColor(const QColor& value) {
    const TrackPointer pTrack = m_pTrackPlayer->getLoadedTrack();
    if (pTrack != nullptr) {
        std::optional<RgbColor> color = RgbColor::fromQColor(value);
        pTrack->setColor(color);
    }
}

QUrl QmlPlayerProxy::getCoverArtUrl() const {
    const TrackPointer pTrack = m_pCurrentTrack;
    if (pTrack == nullptr) {
        return QUrl();
    }

    const CoverInfo coverInfo = pTrack->getCoverInfoWithLocation();
    return AsyncImageProvider::trackLocationToCoverArtUrl(coverInfo.trackLocation);
}

} // namespace qml
} // namespace skin
} // namespace mixxx
