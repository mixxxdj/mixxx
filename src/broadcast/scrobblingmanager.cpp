#include "broadcast/scrobblingmanager.h"

#include <QObject>

#include "util/assert.h"
#include "util/parented_ptr.h"

#ifdef __MPRIS__
#include "broadcast/mpris/mprisservice.h"
#endif
#include "control/controlproxy.h"
#include "engine/enginexfader.h"
#include "mixer/deck.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "moc_scrobblingmanager.cpp"
#include "util/make_const_iterator.h"

namespace {
#ifdef __MPRIS__
const QString kAppGroup = QStringLiteral("[App]");
const ConfigKey kEnabledMpris =
        ConfigKey(
                kAppGroup,
                QStringLiteral("enabled_mpris"));
const bool kEnabledMprisDefault = false;
#endif
} // namespace

TotalVolumeThreshold::TotalVolumeThreshold(QObject* parent, double threshold)
        : m_CPCrossfader("[Master]", "crossfader", parent),
          m_CPXFaderCurve(ConfigKey(EngineXfader::kXfaderConfigKey,
                                  "xFaderCurve"),
                  parent),
          m_CPXFaderCalibration(ConfigKey(EngineXfader::kXfaderConfigKey,
                                        "xFaderCalibration"),
                  parent),
          m_CPXFaderMode(ConfigKey(EngineXfader::kXfaderConfigKey,
                                 "xFaderMode"),
                  parent),
          m_CPXFaderReverse(ConfigKey(EngineXfader::kXfaderConfigKey,
                                    "xFaderReverse"),
                  parent),
          m_volumeThreshold(threshold) {
}

bool TotalVolumeThreshold::isPlayerAudible(BaseTrackPlayer* pPlayer) const {
    VERIFY_OR_DEBUG_ASSERT(pPlayer) {
        return false;
    }
    PollingControlProxy mCOtrackPreGain(pPlayer->getGroup(), "pregain");
    PollingControlProxy mCOtrackVolume(pPlayer->getGroup(), "volume");
    PollingControlProxy mCOdeckOrientation(pPlayer->getGroup(), "orientation");
    int orientation = static_cast<int>(mCOdeckOrientation.get());

    CSAMPLE_GAIN xFaderLeft;
    CSAMPLE_GAIN xFaderRight;

    EngineXfader::getXfadeGains(m_CPCrossfader.get(),
            m_CPXFaderCurve.get(),
            m_CPXFaderCalibration.get(),
            m_CPXFaderMode.get(),
            m_CPXFaderReverse.toBool(),
            &xFaderLeft,
            &xFaderRight);
    double finalVolume = mCOtrackPreGain.get() * mCOtrackVolume.get();
    if (orientation == EngineChannel::LEFT) {
        finalVolume *= xFaderLeft;
    } else if (orientation == EngineChannel::RIGHT) {
        finalVolume *= xFaderRight;
    }
    return finalVolume > m_volumeThreshold;
}

void TotalVolumeThreshold::setVolumeThreshold(double volume) {
    m_volumeThreshold = volume;
}

#ifdef __MPRIS__
template<>
void ScrobblingManager::addScrobblingService<MprisService>() {
    m_pBroadcaster->addScrobblingService<MprisService>(m_pPlayerManager.get(), m_pConfig);
}
#endif

ScrobblingManager::ScrobblingManager(UserSettingsPointer pConfig,
        std::shared_ptr<PlayerManager> pPlayerManager)
        : m_pPlayerManager(pPlayerManager),
          m_pConfig(pConfig),
          m_pBroadcaster(std::make_unique<MetadataBroadcaster>()),
          m_pAudibleStrategy(std::make_unique<TotalVolumeThreshold>(this, 0.20)),
          m_pTimer(make_parented<TrackTimers::GUITickTimer>(this)),
          m_scrobbledAtLeastOnce(false),
          m_GuiTickObject(ConfigKey("[App]", "gui_tick_50ms_period_s")) {
    connect(m_pTimer.get(),
            &TrackTimers::RegularTimer::timeout,
            this,
            &ScrobblingManager::slotCheckAudibleTracks);
    m_GuiTickObject.connectValueChanged(this, &ScrobblingManager::slotGuiTick);
    m_pTimer->start(1000);

#ifdef __MPRIS__
    if (pConfig->getValue(kEnabledMpris, kEnabledMprisDefault)) {
        addScrobblingService<MprisService>();
    }
#endif

    connect(pPlayerManager.get(),
            &PlayerManager::numberOfDecksChanged,
            this,
            &ScrobblingManager::onNumberOfDecksChanged);

    connect(&PlayerInfo::instance(),
            &PlayerInfo::currentPlayingTrackChanged,
            m_pBroadcaster.get(),
            &MetadataBroadcaster::slotNowListening);
}

void ScrobblingManager::setAudibleStrategy(TrackAudibleStrategy* pStrategy) {
    m_pAudibleStrategy.reset(pStrategy);
}

void ScrobblingManager::setTimer(parented_ptr<TrackTimers::RegularTimer> timer) {
    m_pTimer = std::move(timer);
}

void ScrobblingManager::setTrackInfoFactory(
        const std::function<std::shared_ptr<TrackTimingInfo>(TrackPointer)>& factory) {
    m_trackInfoFactory = factory;
}

bool ScrobblingManager::hasScrobbledAnyTrack() const {
    return m_scrobbledAtLeastOnce;
}

void ScrobblingManager::slotTrackPaused(TrackPointer pPausedTrack) {
    if (!pPausedTrack) {
        return;
    }
    if (!m_trackInfoHashDict.contains(pPausedTrack->getId())) {
        m_trackInfoHashDict[pPausedTrack->getId()].init(m_trackInfoFactory, pPausedTrack);
    }

    DEBUG_ASSERT(m_trackInfoHashDict.contains(pPausedTrack->getId()));
    const TrackInfo pausedTrackInfo = m_trackInfoHashDict.value(pPausedTrack->getId());
    for (const auto& playerGroup : pausedTrackInfo.m_players) {
        BaseTrackPlayer* player = m_pPlayerManager->getPlayer(playerGroup);
        DEBUG_ASSERT(player);
        if (!player->isTrackPaused()) {
            return;
        }
    }
    m_trackInfoHashDict[pPausedTrack->getId()].m_trackInfo->pausePlayedTime();
}

void ScrobblingManager::slotTrackResumed(TrackPointer pResumedTrack, const QString& playerGroup) {
    if (!pResumedTrack) {
        return;
    }
    BaseTrackPlayer* player = m_pPlayerManager->getPlayer(playerGroup);
    auto trackId = pResumedTrack->getId();
    if (!m_trackInfoHashDict.contains(trackId)) {
        m_trackInfoHashDict[trackId].init(m_trackInfoFactory, pResumedTrack);
    }

    DEBUG_ASSERT(player && m_trackInfoHashDict.contains(trackId));
    if (m_pAudibleStrategy->isPlayerAudible(player)) {
        TrackInfo info = m_trackInfoHashDict.value(trackId);
        if (info.m_trackInfo->isTimerPaused()) {
            info.m_trackInfo->resumePlayedTime();
        }
    }
}

void ScrobblingManager::slotNewTrackLoaded(TrackPointer pNewTrack, const QString& playerGroup) {
    // Empty player gives a null pointer.
    if (!pNewTrack) {
        return;
    }
    auto trackId = pNewTrack->getId();
    if (!m_trackInfoHashDict.contains(trackId)) {
        m_trackInfoHashDict[trackId].init(m_trackInfoFactory, pNewTrack);
    }
    m_trackInfoHashDict[trackId].m_players.insert(playerGroup);
    connect(m_trackInfoHashDict[trackId].m_trackInfo.get(),
            &TrackTimingInfo::readyToBeScrobbled,
            this,
            &ScrobblingManager::slotReadyToBeScrobbled);
    m_pBroadcaster->newTrackLoaded(pNewTrack);

    auto it = m_trackInfoHashDict.begin();
    while (it != m_trackInfoHashDict.end()) {
        if (it->m_players.contains(playerGroup) && it.key() != trackId) {
            it->m_players.remove(playerGroup);
            if (it->m_players.empty()) {
                it = erase(&m_trackInfoHashDict, it);
                continue;
            }
        }
        ++it;
    }
}

void ScrobblingManager::slotGuiTick(double timeSinceLastTick) {
    for (auto& trackInfo : m_trackInfoHashDict) {
        trackInfo.m_trackInfo->slotGuiTick(timeSinceLastTick);
    }

    MetadataBroadcaster* broadcaster =
            qobject_cast<MetadataBroadcaster*>(m_pBroadcaster.get());
    if (broadcaster) {
        broadcaster->guiTick(timeSinceLastTick);
    }

    TrackTimers::GUITickTimer* timer =
            qobject_cast<TrackTimers::GUITickTimer*>(m_pTimer.get());
    if (timer) {
        timer->slotTick(timeSinceLastTick);
    }
}

void ScrobblingManager::slotReadyToBeScrobbled(TrackPointer pTrack) {
    m_scrobbledAtLeastOnce = true;
    m_pBroadcaster->slotAttemptScrobble(pTrack);
}

void ScrobblingManager::slotCheckAudibleTracks() {
    for (auto& trackInfo : m_trackInfoHashDict) {
        bool audible = false;
        for (const auto& playerGroup : std::as_const(trackInfo.m_players)) {
            BaseTrackPlayer* player = m_pPlayerManager->getPlayer(playerGroup);
            if (m_pAudibleStrategy->isPlayerAudible(player)) {
                audible = true;
                break;
            }
        }
        if (!audible) {
            trackInfo.m_trackInfo->pausePlayedTime();
        } else if (trackInfo.m_trackInfo->isTimerPaused()) {
            trackInfo.m_trackInfo->resumePlayedTime();
        }
    }
    m_pTimer->start(1000);
}

void ScrobblingManager::onNumberOfDecksChanged(int count) {
    for (const auto& connection : std::as_const(m_deckConnections)) {
        disconnect(connection);
    }
    for (int deck = 1; deck <= count; ++deck) {
        Deck* pDeck = m_pPlayerManager->getDeck(deck);
        QString group = pDeck->getGroup();
        m_deckConnections.append(connect(pDeck,
                &Deck::trackPaused,
                this,
                &ScrobblingManager::slotTrackPaused));
        m_deckConnections.append(connect(pDeck,
                &Deck::trackResumed,
                this,
                [this, group](TrackPointer pTrack) -> void { slotTrackResumed(pTrack, group); }));
        m_deckConnections.append(connect(pDeck,
                &Deck::newTrackLoaded,
                this,
                [this, group](TrackPointer pTrack) -> void { slotNewTrackLoaded(pTrack, group); }));
    }
}
