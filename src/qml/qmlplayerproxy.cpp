#include "qml/qmlplayerproxy.h"

#include <QBuffer>

#include "mixer/basetrackplayer.h"
#include "moc_qmlplayerproxy.cpp"
#include "qml/asyncimageprovider.h"

#define PROPERTY_IMPL_GETTER(TYPE, NAME, GETTER)     \
    TYPE QmlPlayerProxy::GETTER() const {            \
        const TrackPointer pTrack = m_pCurrentTrack; \
        if (pTrack == nullptr) {                     \
            return TYPE();                           \
        }                                            \
        return pTrack->GETTER();                     \
    }

#define PROPERTY_IMPL(TYPE, NAME, GETTER, SETTER)    \
    PROPERTY_IMPL_GETTER(TYPE, NAME, GETTER)         \
    void QmlPlayerProxy::SETTER(const TYPE& value) { \
        const TrackPointer pTrack = m_pCurrentTrack; \
        if (pTrack != nullptr) {                     \
            pTrack->SETTER(value);                   \
        }                                            \
    }

namespace mixxx {
namespace qml {

QmlPlayerProxy::QmlPlayerProxy(BaseTrackPlayer* pTrackPlayer, QObject* parent)
        : QObject(parent),
          m_pTrackPlayer(pTrackPlayer),
          m_pBeatsModel(new QmlBeatsModel(this)),
          m_pHotcuesModel(new QmlCuesModel(this))
#ifdef __STEM__
          ,
          m_pStemsModel(std::make_unique<QmlStemsModel>(this))
#endif
{
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
    if (m_pTrackPlayer && m_pTrackPlayer->getLoadedTrack()) {
        slotTrackLoaded(pTrackPlayer->getLoadedTrack());
    }
}

void QmlPlayerProxy::loadTrackFromLocation(const QString& trackLocation, bool play) {
    emit loadTrackFromLocationRequested(trackLocation, play);
}

void QmlPlayerProxy::loadTrackFromLocationUrl(const QUrl& trackLocationUrl, bool play) {
    if (trackLocationUrl.isLocalFile()) {
        loadTrackFromLocation(trackLocationUrl.toLocalFile(), play);
    } else {
        qWarning() << "QmlPlayerProxy: URL" << trackLocationUrl << "is not a local file!";
    }
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
        connect(pTrack.get(),
                &Track::beatsUpdated,
                this,
                &QmlPlayerProxy::slotBeatsChanged);
        connect(pTrack.get(),
                &Track::cuesUpdated,
                this,
                &QmlPlayerProxy::slotHotcuesChanged);
#ifdef __STEM__
        connect(pTrack.get(),
                &Track::stemsUpdated,
                this,
                &QmlPlayerProxy::slotStemsChanged);
#endif
        slotBeatsChanged();
        slotHotcuesChanged();
#ifdef __STEM__
        slotStemsChanged();
#endif
        slotWaveformChanged();
    }
    emit trackChanged();
    emit trackLoaded();
}

void QmlPlayerProxy::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
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
    emit trackLocationUrlChanged();
#ifdef __STEM__
    emit stemsChanged();
#endif

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

void QmlPlayerProxy::slotBeatsChanged() {
    VERIFY_OR_DEBUG_ASSERT(m_pBeatsModel != nullptr) {
        return;
    }

    const TrackPointer pTrack = m_pCurrentTrack;
    if (pTrack) {
        const auto trackEndPosition = mixxx::audio::FramePos{
                pTrack->getDuration() * pTrack->getSampleRate()};
        const auto pBeats = pTrack->getBeats();
        m_pBeatsModel->setBeats(pBeats, trackEndPosition);
    } else {
        m_pBeatsModel->setBeats(nullptr, audio::kStartFramePos);
    }
}

#ifdef __STEM__
void QmlPlayerProxy::slotStemsChanged() {
    VERIFY_OR_DEBUG_ASSERT(m_pStemsModel != nullptr) {
        return;
    }

    const TrackPointer pTrack = m_pCurrentTrack;
    if (pTrack) {
        m_pStemsModel->setStems(pTrack->getStemInfo());
        emit stemsChanged();
    }
}
#endif

void QmlPlayerProxy::slotHotcuesChanged() {
    VERIFY_OR_DEBUG_ASSERT(m_pHotcuesModel != nullptr) {
        return;
    }

    QList<CuePointer> hotcues;

    const TrackPointer pTrack = m_pCurrentTrack;
    if (pTrack) {
        const auto& cuePoints = pTrack->getCuePoints();
        for (const auto& cuePoint : cuePoints) {
            if (cuePoint->getHotCue() == Cue::kNoHotCue)
                continue;
            hotcues.append(cuePoint);
        }
    }
    m_pHotcuesModel->setCues(hotcues);
    emit cuesChanged();
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

bool QmlPlayerProxy::isLoaded() const {
    return m_pCurrentTrack != nullptr;
}

PROPERTY_IMPL(QString, artist, getArtist, setArtist)
PROPERTY_IMPL(QString, title, getTitle, setTitle)
PROPERTY_IMPL(QString, album, getAlbum, setAlbum)
PROPERTY_IMPL(QString, albumArtist, getAlbumArtist, setAlbumArtist)
PROPERTY_IMPL_GETTER(QString, genre, getGenre)
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

QUrl QmlPlayerProxy::getTrackLocationUrl() const {
    const TrackPointer pTrack = m_pCurrentTrack;
    if (pTrack == nullptr) {
        return QUrl();
    }

    return QUrl::fromLocalFile(pTrack->getLocation());
}

} // namespace qml
} // namespace mixxx
