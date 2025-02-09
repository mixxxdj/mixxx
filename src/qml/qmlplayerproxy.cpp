#include "qml/qmlplayerproxy.h"

#include <QBuffer>

#include "mixer/basetrackplayer.h"
#include "moc_qmlplayerproxy.cpp"
#include "qml/asyncimageprovider.h"

#define PROPERTY_IMPL_SETTER(TYPE, NAME, SETTER)     \
    void QmlPlayerProxy::SETTER(const TYPE& value) { \
        const TrackPointer pTrack = m_pCurrentTrack; \
        if (pTrack != nullptr) {                     \
            pTrack->SETTER(value);                   \
        }                                            \
    }

#define PROPERTY_IMPL(TYPE, NAME, GETTER, SETTER)            \
    PROPERTY_IMPL_GETTER(QmlPlayerProxy, TYPE, NAME, GETTER) \
    PROPERTY_IMPL_SETTER(TYPE, NAME, SETTER)

namespace mixxx {
namespace qml {

QmlPlayerProxy::QmlPlayerProxy(BaseTrackPlayer* pTrackPlayer, QObject* parent)
        : JavascriptPlayerProxy(pTrackPlayer, parent),
          m_pBeatsModel(new QmlBeatsModel(this)),
          m_pHotcuesModel(new QmlCuesModel(this))
#ifdef __STEM__
          ,
          m_pStemsModel(std::make_unique<QmlStemsModel>(this))
#endif
{
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

void QmlPlayerProxy::subclassesSlotTrackLoaded() {
    connect(m_pCurrentTrack.get(),
            &Track::commentChanged,
            this,
            &QmlPlayerProxy::commentChanged);
    connect(m_pCurrentTrack.get(),
            &Track::keyChanged,
            this,
            &QmlPlayerProxy::keyTextChanged);
    connect(m_pCurrentTrack.get(),
            &Track::colorUpdated,
            this,
            &QmlPlayerProxy::colorChanged);
    connect(m_pCurrentTrack.get(),
            &Track::waveformUpdated,
            this,
            &QmlPlayerProxy::slotWaveformChanged);
    connect(m_pCurrentTrack.get(),
            &Track::beatsUpdated,
            this,
            &QmlPlayerProxy::slotBeatsChanged);
    connect(m_pCurrentTrack.get(),
            &Track::cuesUpdated,
            this,
            &QmlPlayerProxy::slotHotcuesChanged);
#ifdef __STEM__
    connect(m_pCurrentTrack.get(),
            &Track::stemsUpdated,
            this,
            &QmlPlayerProxy::slotStemsChanged);
#endif
}

void QmlPlayerProxy::subclassesSlotTrackChanged() {
    emit commentChanged();
    emit keyTextChanged();
    emit colorChanged();
    emit coverArtUrlChanged();
    emit trackLocationUrlChanged();
#ifdef __STEM__
    emit stemsChanged();
#endif
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

PROPERTY_IMPL_SETTER(QString, artist, setArtist)
PROPERTY_IMPL_SETTER(QString, title, setTitle)
PROPERTY_IMPL_SETTER(QString, album, setAlbum)
PROPERTY_IMPL_SETTER(QString, albumArtist, setAlbumArtist)
PROPERTY_IMPL_SETTER(QString, composer, setComposer)
PROPERTY_IMPL_SETTER(QString, groupinG, setGrouping)
PROPERTY_IMPL_SETTER(QString, year, setYear)
PROPERTY_IMPL_SETTER(QString, trackNumber, setTrackNumber)
PROPERTY_IMPL_SETTER(QString, trackTotal, setTrackTotal)
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
