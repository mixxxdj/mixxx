// Helper class to have easy access
#include "mixer/playerinfo.h"

#include "engine/channels/enginechannel.h"
#include "engine/enginexfader.h"
#include "mixer/playermanager.h"
#include "moc_playerinfo.cpp"
#include "track/track.h"
#include "util/compatibility/qmutex.h"

namespace {

constexpr int kPlayingDeckUpdateIntervalMillis = 2000;

PlayerInfo* s_pPlayerInfo = nullptr;

const QString kAppGroup = QStringLiteral("[App]");
const QString kMasterGroup = QStringLiteral("[Master]");

} // namespace

PlayerInfo::PlayerInfo()
        : m_xfader(kMasterGroup, QStringLiteral("crossfader")),
          m_numDecks(kAppGroup, QStringLiteral("num_decks")),
          m_numSamplers(kAppGroup, QStringLiteral("num_samplers")),
          m_numPreviewDecks(kAppGroup, QStringLiteral("num_preview_decks")),
          m_currentlyPlayingDeck(-1) {
    startTimer(kPlayingDeckUpdateIntervalMillis);
}

PlayerInfo::~PlayerInfo() {
    m_loadedTrackMap.clear();
    clearControlCache();
}

PlayerInfo& PlayerInfo::create() {
    VERIFY_OR_DEBUG_ASSERT(!s_pPlayerInfo) {
        return *s_pPlayerInfo;
    }
    s_pPlayerInfo = new PlayerInfo();
    return *s_pPlayerInfo;
}

// static
PlayerInfo& PlayerInfo::instance() {
    VERIFY_OR_DEBUG_ASSERT(s_pPlayerInfo) {
        s_pPlayerInfo = new PlayerInfo();
    }
    return *s_pPlayerInfo;
}

// static
void PlayerInfo::destroy() {
    delete s_pPlayerInfo;
    s_pPlayerInfo = nullptr;
}

TrackPointer PlayerInfo::getTrackInfo(const QString& group) {
    const auto locker = lockMutex(&m_mutex);
    return m_loadedTrackMap.value(group);
}

void PlayerInfo::setTrackInfo(const QString& group, const TrackPointer& pTrack) {
    TrackPointer pOld;
    { // Scope
        const auto locker = lockMutex(&m_mutex);
        pOld = m_loadedTrackMap.value(group);
        m_loadedTrackMap.insert(group, pTrack);
    }
    emit trackChanged(group, pTrack, pOld);

    if (pTrack) {
        updateCurrentPlayingDeck();

        int playingDeck = m_currentlyPlayingDeck;
        if (playingDeck >= 0 &&
                group == PlayerManager::groupForDeck(playingDeck)) {
            emit currentPlayingTrackChanged(pTrack);
        }
    }
}

bool PlayerInfo::isTrackLoaded(const TrackPointer& pTrack) const {
    const auto locker = lockMutex(&m_mutex);
    QMapIterator<QString, TrackPointer> it(m_loadedTrackMap);
    while (it.hasNext()) {
        it.next();
        if (it.value() == pTrack) {
            return true;
        }
    }
    return false;
}

QStringList PlayerInfo::getPlayerGroupsWithTracksLoaded(const TrackPointerList& tracks) const {
    const auto locker = lockMutex(&m_mutex);
    QStringList groups;
    QMapIterator<QString, TrackPointer> it(m_loadedTrackMap);
    while (it.hasNext()) {
        it.next();
        TrackPointer pLoadedTrack = it.value();
        if (pLoadedTrack && tracks.contains(pLoadedTrack)) {
            groups.append(it.key());
        }
    }
    return groups;
}

QMap<QString, TrackPointer> PlayerInfo::getLoadedTracks() {
    const auto locker = lockMutex(&m_mutex);
    QMap<QString, TrackPointer> ret = m_loadedTrackMap;
    return ret;
}

bool PlayerInfo::isFileLoaded(const QString& track_location) const {
    const auto locker = lockMutex(&m_mutex);
    QMapIterator<QString, TrackPointer> it(m_loadedTrackMap);
    while (it.hasNext()) {
        it.next();
        TrackPointer pTrack = it.value();
        if (pTrack) {
            if (pTrack->getLocation() == track_location) {
                return true;
            }
        }
    }
    return false;
}

void PlayerInfo::timerEvent(QTimerEvent* pTimerEvent) {
    Q_UNUSED(pTimerEvent);
    updateCurrentPlayingDeck();
}

void PlayerInfo::updateCurrentPlayingDeck() {
    auto locker = lockMutex(&m_mutex);

    double maxVolume = 0;
    int maxDeck = -1;

    CSAMPLE_GAIN xfl, xfr;
    // TODO: supply correct parameters to the function. If the hamster style
    // for the crossfader is enabled, the result is currently wrong.
    EngineXfader::getXfadeGains(m_xfader.get(),
            1.0,
            0.0,
            MIXXX_XFADER_ADDITIVE,
            false,
            &xfl,
            &xfr);

    for (int i = 0; i < numDecks(); ++i) {
        DeckControls* pDc = getDeckControls(i);

        if (pDc->m_play.get() == 0.0) {
            continue;
        }

        if (pDc->m_pregain.get() <= 0.25) {
            continue;
        }

        double fvol = pDc->m_volume.get();
        if (fvol == 0.0) {
            continue;
        }

        const auto orient = static_cast<int>(pDc->m_orientation.get());
        double xfvol;
        if (orient == EngineChannel::LEFT) {
            xfvol = xfl;
        } else if (orient == EngineChannel::RIGHT) {
            xfvol = xfr;
        } else {
            xfvol = 1.0;
        }

        double dvol = fvol * xfvol;
        if (dvol > maxVolume) {
            maxDeck = i;
            maxVolume = dvol;
        }
    }
    locker.unlock();

    int oldDeck = m_currentlyPlayingDeck.fetchAndStoreRelease(maxDeck);
    if (maxDeck != oldDeck) {
        emit currentPlayingDeckChanged(maxDeck);
        // Note: When starting Auto-DJ "play" might be processed before a new
        // is track is fully loaded. currentPlayingTrackChanged() is then emitted
        // after setTrackInfo().
        emit currentPlayingTrackChanged(getCurrentPlayingTrack());
    }
}

int PlayerInfo::getCurrentPlayingDeck() {
    updateCurrentPlayingDeck();
    return m_currentlyPlayingDeck;
}

TrackPointer PlayerInfo::getCurrentPlayingTrack() {
    int deck = getCurrentPlayingDeck();
    if (deck >= 0) {
        return getTrackInfo(PlayerManager::groupForDeck(deck));
    }
    return TrackPointer();
}

PlayerInfo::DeckControls* PlayerInfo::getDeckControls(int i) {
    if (m_deckControlList.count() == i) {
        QString group = PlayerManager::groupForDeck(i);
        m_deckControlList.append(new DeckControls(group));
    }
    return m_deckControlList[i];
}

void PlayerInfo::clearControlCache() {
    for (int i = 0; i < m_deckControlList.count(); ++i) {
        delete m_deckControlList[i];
    }
    m_deckControlList.clear();
}

int PlayerInfo::numDecks() const {
    return static_cast<int>(m_numDecks.get());
}

int PlayerInfo::numPreviewDecks() const {
    return static_cast<int>(m_numPreviewDecks.get());
}

int PlayerInfo::numSamplers() const {
    return static_cast<int>(m_numSamplers.get());
}
