#include "qml/qmltrackproxy.h"

#include <QBuffer>

#include "mixer/basetrackplayer.h"
#include "moc_qmltrackproxy.cpp"
#include "qml/asyncimageprovider.h"
#include "track/track.h"
#include "util/parented_ptr.h"

#define PROPERTY_IMPL_GETTER(TYPE, NAME, GETTER) \
    TYPE QmlTrackProxy::GETTER() const {         \
        const TrackPointer pTrack = m_pTrack;    \
        if (pTrack == nullptr) {                 \
            return TYPE();                       \
        }                                        \
        return pTrack->GETTER();                 \
    }

#define PROPERTY_IMPL(TYPE, NAME, GETTER, SETTER)   \
    PROPERTY_IMPL_GETTER(TYPE, NAME, GETTER)        \
    void QmlTrackProxy::SETTER(const TYPE& value) { \
        const TrackPointer pTrack = m_pTrack;       \
        if (pTrack != nullptr) {                    \
            pTrack->SETTER(value);                  \
        }                                           \
    }

namespace mixxx {
namespace qml {

QmlTrackProxy::QmlTrackProxy(TrackPointer track, QObject* parent)
        : QObject(parent),
          m_pTrack(track),
          m_pBeatsModel(make_parented<QmlBeatsModel>(this)),
          m_pHotcuesModel(make_parented<QmlCuesModel>(this))
#ifdef __STEM__
          ,
          m_pStemsModel(make_parented<QmlStemsModel>(this))
#endif
{
    if (m_pTrack == nullptr) {
        return;
    }
    connect(m_pTrack.get(),
            &Track::artistChanged,
            this,
            &QmlTrackProxy::artistChanged);
    connect(m_pTrack.get(),
            &Track::titleChanged,
            this,
            &QmlTrackProxy::titleChanged);
    connect(m_pTrack.get(),
            &Track::albumChanged,
            this,
            &QmlTrackProxy::albumChanged);
    connect(m_pTrack.get(),
            &Track::albumArtistChanged,
            this,
            &QmlTrackProxy::albumArtistChanged);
    connect(m_pTrack.get(),
            &Track::genreChanged,
            this,
            &QmlTrackProxy::genreChanged);
    connect(m_pTrack.get(),
            &Track::composerChanged,
            this,
            &QmlTrackProxy::composerChanged);
    connect(m_pTrack.get(),
            &Track::groupingChanged,
            this,
            &QmlTrackProxy::groupingChanged);
    connect(m_pTrack.get(),
            &Track::yearChanged,
            this,
            &QmlTrackProxy::yearChanged);
    connect(m_pTrack.get(),
            &Track::trackNumberChanged,
            this,
            &QmlTrackProxy::trackNumberChanged);
    connect(m_pTrack.get(),
            &Track::trackTotalChanged,
            this,
            &QmlTrackProxy::trackTotalChanged);
    connect(m_pTrack.get(),
            &Track::commentChanged,
            this,
            &QmlTrackProxy::commentChanged);
    connect(m_pTrack.get(),
            &Track::keyChanged,
            this,
            &QmlTrackProxy::keyTextChanged);
    connect(m_pTrack.get(),
            &Track::colorUpdated,
            this,
            &QmlTrackProxy::colorChanged);
    connect(m_pTrack.get(),
            &Track::beatsUpdated,
            this,
            &QmlTrackProxy::slotBeatsChanged);
    connect(m_pTrack.get(),
            &Track::cuesUpdated,
            this,
            &QmlTrackProxy::slotHotcuesChanged);
    connect(m_pTrack.get(),
            &Track::durationChanged,
            this,
            &QmlTrackProxy::durationChanged);
#ifdef __STEM__
    connect(m_pTrack.get(),
            &Track::stemsUpdated,
            this,
            &QmlTrackProxy::slotStemsChanged);
#endif
    slotBeatsChanged();
    slotHotcuesChanged();
#ifdef __STEM__
    slotStemsChanged();
#endif
}

void QmlTrackProxy::slotBeatsChanged() {
    VERIFY_OR_DEBUG_ASSERT(m_pBeatsModel) {
        return;
    }

    const TrackPointer pTrack = m_pTrack;
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
void QmlTrackProxy::slotStemsChanged() {
    VERIFY_OR_DEBUG_ASSERT(m_pStemsModel) {
        return;
    }

    if (m_pTrack) {
        m_pStemsModel->setStems(m_pTrack->getStemInfo());
        emit stemsChanged();
    }
}
#endif

void QmlTrackProxy::slotHotcuesChanged() {
    VERIFY_OR_DEBUG_ASSERT(m_pHotcuesModel) {
        return;
    }

    QList<CuePointer> hotcues;

    if (m_pTrack) {
        const auto& cuePoints = m_pTrack->getCuePoints();
        for (const auto& cuePoint : cuePoints) {
            if (cuePoint->getHotCue() == Cue::kNoHotCue) {
                continue;
            }
            hotcues.append(cuePoint);
        }
    }
    m_pHotcuesModel->setCues(hotcues);
    emit cuesChanged();
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

QColor QmlTrackProxy::getColor() const {
    if (m_pTrack == nullptr) {
        return QColor();
    }
    return RgbColor::toQColor(m_pTrack->getColor());
}

double QmlTrackProxy::getDuration() const {
    if (m_pTrack == nullptr) {
        return -1;
    }
    return m_pTrack->getDuration();
}

int QmlTrackProxy::getSampleRate() const {
    if (m_pTrack == nullptr) {
        return 0;
    }
    return m_pTrack->getSampleRate();
}

void QmlTrackProxy::setColor(const QColor& value) {
    if (m_pTrack) {
        std::optional<RgbColor> color = RgbColor::fromQColor(value);
        m_pTrack->setColor(color);
    }
}

int QmlTrackProxy::getStars() const {
    if (m_pTrack == nullptr) {
        return -1;
    }
    return m_pTrack->getRating();
}

void QmlTrackProxy::setStars(int value) {
    if (m_pTrack && value <= mixxx::TrackRecord::kMaxRating &&
            value >= mixxx::TrackRecord::kMinRating) {
        m_pTrack->setRating(value);
    }
}

QUrl QmlTrackProxy::getCoverArtUrl() const {
    if (m_pTrack == nullptr) {
        return QUrl();
    }

    const CoverInfo coverInfo = m_pTrack->getCoverInfoWithLocation();
    return AsyncImageProvider::trackLocationToCoverArtUrl(coverInfo.trackLocation);
}

QUrl QmlTrackProxy::getTrackLocationUrl() const {
    if (m_pTrack == nullptr) {
        return QUrl();
    }

    return QUrl::fromLocalFile(m_pTrack->getLocation());
}

} // namespace qml
} // namespace mixxx
