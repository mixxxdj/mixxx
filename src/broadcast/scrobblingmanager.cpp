#include "broadcast/scrobblingmanager.h"

#include <QObject>

#include "broadcast/filelistener.h"
#include "broadcast/scrobblingmanager.h"
#include "control/controlproxy.h"
#include "engine/enginexfader.h"
#include "mixer/deck.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "moc_scrobblingmanager.cpp"

ScrobblingManager::ScrobblingManager(PlayerManager* pManager)
        : m_pManager(pManager),
          m_broadcaster(),
          m_CPGuiTick("[Master]", "guiTick50ms", this),
          m_CPCrossfader("[Master]", "crossfader", this),
          m_CPXFaderCurve(ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderCurve"), this),
          m_CPXFaderCalibration(ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderCalibration"), this),
          m_CPXFaderMode(ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderMode"), this),
          m_CPXFaderReverse(ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderReverse"), this) {
    m_CPGuiTick.connectValueChanged(this, &ScrobblingManager::slotGuiTick);
    connect(&PlayerInfo::instance(), &PlayerInfo::currentPlayingTrackChanged, &m_broadcaster, &MetadataBroadcaster::slotNowListening);
    m_broadcaster
            .addNewScrobblingService(new FileListener("nowListening.txt"));
    startTimer(1000);
}

void ScrobblingManager::slotTrackPaused(TrackPointer pPausedTrack) {
    QMutexLocker locker(&m_mutex);
    bool allPaused = true;
    TrackInfo* pPausedTrackInfo = nullptr;
    for (TrackInfo* pTrackInfo : m_trackList) {
        VERIFY_OR_DEBUG_ASSERT(pTrackInfo) {
            return;
        }
        if (pTrackInfo->m_pTrack == pPausedTrack) {
            pPausedTrackInfo = pTrackInfo;
            for (QString playerGroup : pTrackInfo->m_players) {
                BaseTrackPlayer* pPlayer = m_pManager->getPlayer(playerGroup);
                if (!pPlayer->isTrackPaused())
                    allPaused = false;
            }
            break;
        }
    }
    if (allPaused && pPausedTrackInfo) {
        pPausedTrackInfo->m_trackInfo.pausePlayedTime();
    }
}

void ScrobblingManager::slotTrackResumed(TrackPointer pResumedTrack) {
    BaseTrackPlayer* pPlayer = qobject_cast<Deck*>(sender());
    VERIFY_OR_DEBUG_ASSERT(pPlayer) {
        return;
    }
    if (isTrackAudible(pResumedTrack, pPlayer)) {
        QMutexLocker locker(&m_mutex);
        for (TrackInfo* pTrackInfo : m_trackList) {
            VERIFY_OR_DEBUG_ASSERT(pTrackInfo) {
                return;
            }
            if (pTrackInfo->m_pTrack == pResumedTrack &&
                    pTrackInfo->m_trackInfo.isTimerPaused()) {
                pTrackInfo->m_trackInfo.resumePlayedTime();
                break;
            }
        }
    }
}

void ScrobblingManager::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pNewTrack);
    BaseTrackPlayer* pSourcePlayer =
            qobject_cast<BaseTrackPlayer*>(sender());
    DEBUG_ASSERT(pSourcePlayer);
    if (pOldTrack) {
        m_tracksToBeReset.append(TrackToBeReset(pOldTrack,
                pSourcePlayer->getGroup()));
    }
}

void ScrobblingManager::slotNewTrackLoaded(TrackPointer pNewTrack) {
    //Empty player gives a null pointer.
    if (!pNewTrack)
        return;
    BaseTrackPlayer* pPlayer = qobject_cast<BaseTrackPlayer*>(sender());
    VERIFY_OR_DEBUG_ASSERT(pPlayer) {
        return;
    }
    QMutexLocker locker(&m_mutex);
    bool trackAlreadyAdded = false;
    for (TrackInfo* pTrackInfo : m_trackList) {
        if (pTrackInfo->m_pTrack == pNewTrack) {
            pTrackInfo->m_players.append(pPlayer->getGroup());
            trackAlreadyAdded = true;
            break;
        }
    }
    if (!trackAlreadyAdded) {
        TrackInfo* pNewTrackInfo = new TrackInfo(pNewTrack);
        pNewTrackInfo->m_players.append(pPlayer->getGroup());
        m_trackList.append(pNewTrackInfo);
        connect(&m_trackList.last()->m_trackInfo, SIGNAL(readyToBeScrobbled(TrackPointer)), &m_broadcaster, SLOT(slotAttemptScrobble(TrackPointer)));
        m_broadcaster.newTrackLoaded(pNewTrack);
    }
    // A new track has been loaded so must unload old one.
    resetTracks();
}

void ScrobblingManager::slotPlayerEmpty() {
    QMutexLocker locker(&m_mutex);
    resetTracks();
}

void ScrobblingManager::resetTracks() {
    for (TrackToBeReset candidateTrack : m_tracksToBeReset) {
        QLinkedList<TrackInfo*>::Iterator trackListIterator =
                m_trackList.begin();
        for (; trackListIterator != m_trackList.end(); ++trackListIterator) {
            TrackInfo* pTrackInfo = *trackListIterator;
            if (pTrackInfo->m_pTrack == candidateTrack.m_pTrack) {
                if (!pTrackInfo->m_players.contains(candidateTrack.m_playerGroup)) {
                    qDebug() << "Track doesn't contain player"
                                "yet is requested for deletion.";
                    break;
                }
                //Load error, stray from engine buffer.
                BaseTrackPlayer* pPlayer =
                        m_pManager->getPlayer(candidateTrack.m_playerGroup);
                if (pPlayer->getLoadedTrack() ==
                        candidateTrack.m_pTrack) {
                    break;
                }
                //Delete player from player list.
                QLinkedList<QString>::iterator it =
                        pTrackInfo->m_players.begin();
                while (it != pTrackInfo->m_players.end() &&
                        *it != candidateTrack.m_playerGroup) {
                    ++it;
                }
                if (*it == candidateTrack.m_playerGroup) {
                    pTrackInfo->m_players.erase(it);
                }
                // If player list is empty, notify and erase.
                if (pTrackInfo->m_players.empty()) {
                    pTrackInfo->m_trackInfo.pausePlayedTime();
                    pTrackInfo->m_trackInfo.resetPlayedTime();
                    delete pTrackInfo;
                    m_trackList.erase(trackListIterator);
                    m_broadcaster.trackUnloaded(pTrackInfo->m_pTrack);
                }
                break;
            }
        }
    }
}

bool ScrobblingManager::isTrackAudible(TrackPointer pTrack, BaseTrackPlayer* pPlayer) {
    if (pPlayer->getLoadedTrack() != pTrack) {
        qDebug() << "Track can't be audible because is not in player";
        return false;
    }
    return getPlayerVolume(pPlayer) >= 0.20;
}

double ScrobblingManager::getPlayerVolume(BaseTrackPlayer* pPlayer) {
    double finalVolume;
    ControlProxy trackPreGain(pPlayer->getGroup(), "pregain", this);
    double preGain = trackPreGain.get();
    ControlProxy trackVolume(pPlayer->getGroup(), "volume", this);
    double volume = trackVolume.get();
    ControlProxy deckOrientation(pPlayer->getGroup(), "orientation", this);
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
    return finalVolume;
}

void ScrobblingManager::slotGuiTick(double timeSinceLastTick) {
    for (TrackInfo* pTrackInfo : m_trackList) {
        pTrackInfo->m_trackInfo.slotGuiTick(timeSinceLastTick);
    }
    m_broadcaster.slotGuiTick(timeSinceLastTick);
}

void ScrobblingManager::timerEvent(QTimerEvent* pTimerEvent) {
    Q_UNUSED(pTimerEvent);
    for (TrackInfo* pTrackInfo : m_trackList) {
        bool inaudible = true;
        for (QString playerGroup : pTrackInfo->m_players) {
            BaseTrackPlayer* pPlayer = m_pManager->getPlayer(playerGroup);
            if (isTrackAudible(pTrackInfo->m_pTrack, pPlayer)) {
                inaudible = false;
                break;
            }
        }
        if (inaudible) {
            pTrackInfo->m_trackInfo.pausePlayedTime();
        } else if (pTrackInfo->m_trackInfo.isTimerPaused()) {
            pTrackInfo->m_trackInfo.resumePlayedTime();
        }
    }
}
