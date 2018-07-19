#include "broadcast/scrobblingmanager.h"

#include <QObject>

#include "broadcast/filelistener/filelistener.h"
#include "broadcast/scrobblingmanager.h"
#include "control/controlproxy.h"
#include "engine/enginexfader.h"
#include "mixer/deck.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "moc_scrobblingmanager.cpp"

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
          m_pParent(parent),
          m_volumeThreshold(threshold) {
}

bool TotalVolumeThreshold::isTrackAudible(TrackPointer pTrack,
        BaseTrackPlayer* pPlayer) const {
    DEBUG_ASSERT(pPlayer);
    Q_UNUSED(pTrack);
    double finalVolume;
    ControlProxy trackPreGain(pPlayer->getGroup(), "pregain", m_pParent);
    double preGain = trackPreGain.get();
    ControlProxy trackVolume(pPlayer->getGroup(), "volume", m_pParent);
    double volume = trackVolume.get();
    ControlProxy deckOrientation(pPlayer->getGroup(), "orientation", m_pParent);
    int orientation = static_cast<int>(deckOrientation.get());

    CSAMPLE_GAIN xFaderLeft;
    CSAMPLE_GAIN xFaderRight;

    EngineXfader::getXfadeGains(m_CPCrossfader.get(),
            m_CPXFaderCurve.get(),
            m_CPXFaderCalibration.get(),
            m_CPXFaderMode.get(),
            m_CPXFaderReverse.toBool(),
            &xFaderLeft,
            &xFaderRight);
    finalVolume = preGain * volume;
    if (orientation == EngineChannel::LEFT)
        finalVolume *= xFaderLeft;
    else if (orientation == EngineChannel::RIGHT)
        finalVolume *= xFaderRight;
    return finalVolume > m_volumeThreshold;
}

void TotalVolumeThreshold::setVolumeThreshold(double volume) {
    m_volumeThreshold = volume;
}

ScrobblingManager::ScrobblingManager(PlayerManagerInterface* manager)
        : m_pManager(manager),
          m_pAudibleStrategy(new TotalVolumeThreshold(this, 0.20)),
          m_pTimer(new TrackTimers::GUITickTimer),
          m_scrobbledAtLeastOnce(false),
          m_GuiTickObject(ConfigKey("[Master]", "guiTick50ms")) {
    connect(m_pTimer.get(), SIGNAL(timeout()), this, SLOT(slotCheckAudibleTracks()));
    m_GuiTickObject.connectValueChanged(this, &ScrobblingManager::slotGuiTick);
    m_pTimer->start(1000);
}

void ScrobblingManager::setAudibleStrategy(TrackAudibleStrategy* pStrategy) {
    m_pAudibleStrategy.reset(pStrategy);
}

void ScrobblingManager::setMetadataBroadcaster(MetadataBroadcasterInterface* pBroadcast) {
    m_pBroadcaster.reset(pBroadcast);
    connect(&PlayerInfo::instance(), SIGNAL(currentPlayingTrackChanged(TrackPointer)), m_pBroadcaster.get(), SLOT(slotNowListening(TrackPointer)));
}

void ScrobblingManager::setTimer(TrackTimers::RegularTimer* timer) {
    m_pTimer.reset(timer);
}

void ScrobblingManager::setTrackInfoFactory(
        const std::function<std::shared_ptr<TrackTimingInfo>(TrackPointer)>& factory) {
    m_trackInfoFactory = factory;
}

bool ScrobblingManager::hasScrobbledAnyTrack() const {
    return m_scrobbledAtLeastOnce;
}

void ScrobblingManager::slotTrackPaused(TrackPointer pPausedTrack) {
    bool pausedInAllDecks = true;
    auto pausedTrackIterator = m_trackList.end();
    for (auto it = m_trackList.begin(); it != m_trackList.end(); ++it) {
        auto& trackInfoPtr = *it;
        std::shared_ptr<Track> pTrack = trackInfoPtr->m_pTrack.lock();
        if (pTrack && pTrack == pPausedTrack) {
            pausedTrackIterator = it;
            for (const QString& playerGroup : trackInfoPtr->m_players) {
                BaseTrackPlayer* player = m_pManager->getPlayer(playerGroup);
                if (!player->isTrackPaused())
                    pausedInAllDecks = false;
            }
            break;
        }
    }
    if (pausedInAllDecks && pausedTrackIterator != m_trackList.end()) {
        (*pausedTrackIterator)->m_trackInfo->pausePlayedTime();
        bool allTracksPaused = true;
        for (const auto& trackInfoPtr : m_trackList) {
            for (const QString& player : trackInfoPtr->m_players) {
                if (!m_pManager->getPlayer(player)->isTrackPaused()) {
                    allTracksPaused = false;
                    break;
                }
            }
            if (!allTracksPaused) {
                break;
            }
        }
        if (allTracksPaused) {
            m_pBroadcaster->slotAllTracksPaused();
        }
    }
}

void ScrobblingManager::slotTrackResumed(TrackPointer pResumedTrack, const QString& playerGroup) {
    BaseTrackPlayer* player = m_pManager->getPlayer(playerGroup);
    DEBUG_ASSERT(player);
    if (m_pAudibleStrategy->isTrackAudible(pResumedTrack, player)) {
        for (auto& trackInfoPtr : m_trackList) {
            std::shared_ptr<Track> pTrack = trackInfoPtr->m_pTrack.lock();
            if (pTrack == pResumedTrack &&
                    trackInfoPtr->m_trackInfo->isTimerPaused()) {
                trackInfoPtr->m_trackInfo->resumePlayedTime();
                break;
            }
        }
    }
}

void ScrobblingManager::slotLoadingTrack(TrackPointer pNewTrack,
        TrackPointer pOldTrack,
        const QString& playerGroup) {
    Q_UNUSED(pNewTrack);
    if (pOldTrack) {
        m_tracksToBeReset.append(TrackToBeReset(pOldTrack,
                playerGroup));
    }
}

void ScrobblingManager::slotNewTrackLoaded(TrackPointer pNewTrack, const QString& playerGroup) {
    //Empty player gives a null pointer.
    if (!pNewTrack)
        return;
    BaseTrackPlayer* pPlayer = m_pManager->getPlayer(playerGroup);
    DEBUG_ASSERT(pPlayer);
    bool trackAlreadyAdded = false;
    for (auto& trackInfoPtr : m_trackList) {
        std::shared_ptr<Track> pTrack = trackInfoPtr->m_pTrack.lock();
        if (pTrack && pTrack == pNewTrack) {
            trackInfoPtr->m_players.append(pPlayer->getGroup());
            trackAlreadyAdded = true;
            break;
        }
    }
    if (!trackAlreadyAdded) {
        TrackInfo* pNewTrackInfo = new TrackInfo(pNewTrack);
        pNewTrackInfo->m_players.append(pPlayer->getGroup());
        if (m_trackInfoFactory) {
            pNewTrackInfo->m_trackInfo = m_trackInfoFactory(pNewTrack);
        }
        connect(m_trackList.back()->m_trackInfo.get(), &TrackTimingInfo::readyToBeScrobbled, this, &ScrobblingManager::slotReadyToBeScrobbled);
        m_trackList.push_back(std::move(std::unique_ptr<TrackInfo>(pNewTrackInfo)));
        m_pBroadcaster->newTrackLoaded(pNewTrack);
    }
    // A new track has been loaded so must unload old one.
    resetTracks();
}

void ScrobblingManager::slotPlayerEmpty() {
    resetTracks();
}

void ScrobblingManager::resetTracks() {
    auto candidateIt = m_tracksToBeReset.begin();
    while (candidateIt != m_tracksToBeReset.end()) {
        TrackToBeReset candidateTrack = *candidateIt;
        auto it = m_trackList.begin();
        while (it != m_trackList.end()) {
            auto& trackInfoPtr = *it;
            std::shared_ptr<Track> pActualTrack = trackInfoPtr->m_pTrack.lock();
            std::shared_ptr<Track> pCandidateTrack = candidateTrack.m_pTrack.lock();
            if (pActualTrack && pCandidateTrack && pActualTrack == pCandidateTrack) {
                if (playerNotInTrackList(trackInfoPtr->m_players,
                            candidateTrack.m_playerGroup) ||
                        isStrayFromEngine(trackInfoPtr->m_pTrack.lock(),
                                candidateTrack.m_playerGroup)) {
                    break;
                }
                deletePlayerFromList(candidateTrack.m_playerGroup,
                        trackInfoPtr->m_players);
                if (trackInfoPtr->m_players.empty()) {
                    deleteTrackInfoAndNotify(it);
                }
                break;
            }
            ++it;
        }
        if (it == m_trackList.end()) {
            qDebug() << "Added track to reset that is not in the tracked list.";
        }
        candidateIt = m_tracksToBeReset.erase(candidateIt);
    }
}

bool ScrobblingManager::isStrayFromEngine(TrackPointer pTrack,
        const QString& group) const {
    BaseTrackPlayer* player = m_pManager->getPlayer(group);
    return player->getLoadedTrack() == pTrack;
}

bool ScrobblingManager::playerNotInTrackList(const QLinkedList<QString>& list,
        const QString& group) const {
    if (!list.contains(group)) {
        qDebug() << "Player added to reset yet not in track list";
        return true;
    }
    return false;
}

void ScrobblingManager::deletePlayerFromList(
        const QString& player,
        QLinkedList<QString>& list) {
    QLinkedList<QString>::iterator it;
    for (it = list.begin();
            it != list.end() && *it != player;
            ++it)
        ;
    if (*it == player) {
        list.erase(it);
    }
}

void ScrobblingManager::deleteTrackInfoAndNotify(trackInfoPointerListIterator& it) {
    (*it)->m_trackInfo->pausePlayedTime();
    (*it)->m_trackInfo->resetPlayedTime();
    m_pBroadcaster->trackUnloaded(TrackPointer());
    m_trackList.erase(it);
}

void ScrobblingManager::slotGuiTick(double timeSinceLastTick) {
    for (auto& trackInfo : m_trackList) {
        trackInfo->m_trackInfo->slotGuiTick(timeSinceLastTick);
    }

    MetadataBroadcaster* broadcaster =
            qobject_cast<MetadataBroadcaster*>(m_pBroadcaster.get());
    if (broadcaster)
        broadcaster->guiTick(timeSinceLastTick);

    TrackTimers::GUITickTimer* timer =
            qobject_cast<TrackTimers::GUITickTimer*>(m_pTimer.get());
    if (timer)
        timer->slotTick(timeSinceLastTick);
}

void ScrobblingManager::slotReadyToBeScrobbled(TrackPointer pTrack) {
    m_scrobbledAtLeastOnce = true;
    m_pBroadcaster->slotAttemptScrobble(pTrack);
}

void ScrobblingManager::slotCheckAudibleTracks() {
    for (auto& pTrackInfo : m_trackList) {
        bool inaudible = true;
        for (QString playerGroup : pTrackInfo->m_players) {
            BaseTrackPlayer* pPlayer = m_pManager->getPlayer(playerGroup);
            std::shared_ptr<Track> pTrack = pTrackInfo->m_pTrack.lock();
            if (pTrack &&
                    m_pAudibleStrategy->isTrackAudible(pTrack, pPlayer)) {
                inaudible = false;
                break;
            }
        }
        if (inaudible) {
            pTrackInfo->m_trackInfo->pausePlayedTime();
        } else if (pTrackInfo->m_trackInfo->isTimerPaused()) {
            pTrackInfo->m_trackInfo->resumePlayedTime();
        }
    }
    m_pTimer->start(1000);
}
