#include "broadcast/scrobblingmanager.h"

#include <QObject>

#include "control/controlproxy.h"
#include "engine/enginexfader.h"
#include "mixer/deck.h"
#include "moc_scrobblingmanager.cpp"

ScrobblingManager::ScrobblingManager()
        : m_CPGuiTick("[Master]", "guiTick50ms", this),
          m_CPCrossfader("[Master]", "crossfader", this),
          m_CPXFaderCurve(ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderCurve"), this),
          m_CPXFaderCalibration(ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderCalibration"), this),
          m_CPXFaderMode(ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderMode"), this),
          m_CPXFaderReverse(ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderReverse"), this) {
    m_CPGuiTick.connectValueChanged(this, &ScrobblingManager::slotGuiTick);
    startTimer(1000);
}

void ScrobblingManager::slotTrackPaused(TrackPointer pPausedTrack) {
    //Extra functional decision to only track main decks.            
    Deck* pSourceDeck = qobject_cast<Deck*>(sender());    
    if (pSourceDeck == 0) {
        qDebug() << "Didn't load track in a deck yet scrobbling "
                    "received paused signal.";
        return;
    }
    // QMutexLocker locker(&m_mutex);
    bool allPaused = true;
    TrackInfo* pPausedTrackInfo;
    for (TrackInfo* pTrackInfo : m_trackList) {
        if (pTrackInfo->m_pTrack == pPausedTrack) {
            pPausedTrackInfo = pTrackInfo;
            for (BaseTrackPlayer* pPlayer : pTrackInfo->m_players) {
                BaseTrackPlayerImpl* pPlayerImpl = 
                    qobject_cast<BaseTrackPlayerImpl*>(pPlayer);
                if (!pPlayerImpl) {
                    qDebug() << "Track player interface isn't a "
                                "BaseTrackPlayerImpl";
                    return;
                }
                if (!pPlayerImpl->isTrackPaused())
                    allPaused = false;
            }
            break;
        }                   
    }
    if (allPaused) {
        pPausedTrackInfo->m_trackInfo.pausePlayedTime();
    }
}

void ScrobblingManager::slotTrackResumed(TrackPointer pResumedTrack) {
    //Extra functional decision to only track main decks.
    Deck* pSourceDeck = qobject_cast<Deck*>(sender());    
    if (pSourceDeck == 0) {
        qDebug() << "Didn't load track in a deck yet scrobbling "
                    "received resumed signal.";
        return;
    }
    if (isTrackAudible(pResumedTrack, pSourceDeck)) {
        // QMutexLocker locker(&m_mutex);
        for (TrackInfo* pTrackInfo : m_trackList) {
            if (pTrackInfo->m_pTrack == pResumedTrack) {
                pTrackInfo->m_trackInfo.resumePlayedTime();
                break;
            }
        }
    }
}

void ScrobblingManager::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pNewTrack);
    // Extra functional decision to only track main decks.
    Deck* pSourceDeck = qobject_cast<Deck*>(sender());    
    if (pSourceDeck == 0) {
        qDebug() << "Didn't load track in a deck yet scrobbling "
                    "received loading signal.";
        return;
    }
    if (pOldTrack) {
        m_tracksToBeReset.append(TrackToBeReset(pOldTrack, pSourceDeck));
    }
}

void ScrobblingManager::slotNewTrackLoaded(TrackPointer pNewTrack) {
    // Empty deck gives a null pointer.
    if (!pNewTrack)
        return;
    //Extra functional decision to only track main decks.        
    Deck* pNewDeck = qobject_cast<Deck*>(sender());    
    if (pNewDeck == 0) {
        qDebug() << "Didn't load track in a deck yet scrobbling "
                    "received loaded signal.";
        return;
    }
    // QMutexLocker locker(&m_mutex); 
    bool trackAlreadyAdded = false;
    for (TrackInfo* pTrackInfo : m_trackList) {
        if (pTrackInfo->m_pTrack == pNewTrack) {
        	pTrackInfo->m_players.append(pNewDeck);
            trackAlreadyAdded = true;
            break;
        }
    }
    if (!trackAlreadyAdded) {
        TrackInfo* pNewTrackInfo = new TrackInfo(pNewTrack, pNewDeck);
        pNewTrackInfo->m_players.append(pNewDeck);
        m_trackList.append(pNewTrackInfo);
        connect(&m_trackList.last()->m_trackInfo,SIGNAL(readyToBeScrobbled(TrackPointer)),
                this,SLOT(slotReadyToBeScrobbled(TrackPointer)));
    }
    // A new track has been loaded so must unload old one.
    resetTracks();
}

void ScrobblingManager::slotPlayerEmpty() {
    // QMutexLocker locker(&m_mutex);
    resetTracks();
}

void ScrobblingManager::resetTracks() {
    for (TrackToBeReset candidateTrack : m_tracksToBeReset) {
        for (TrackInfo* pTrackInfo : m_trackList) {
            if (pTrackInfo->m_pTrack == candidateTrack.m_pTrack) {
                if (!pTrackInfo->m_players.contains(candidateTrack.m_pPlayer)) {
                    qDebug() << "Track doesn't contain player"
                                "yet is requested for deletion.";
                    return;
                }
                // Load error, stray from engine buffer.
                if (candidateTrack.m_pPlayer->getLoadedTrack() ==
                    candidateTrack.m_pTrack) 
                    break;                    
                QLinkedList<BaseTrackPlayer*>::iterator it = 
                    pTrackInfo->m_players.begin();
                while (it != pTrackInfo->m_players.end()) {
                    if (*it == candidateTrack.m_pPlayer) {
                        pTrackInfo->m_players.erase(it);
                    }
                }
                if (pTrackInfo->m_players.empty()) {
                    pTrackInfo->m_trackInfo.pausePlayedTime();
                    pTrackInfo->m_trackInfo.resetPlayedTime();
                    delete pTrackInfo;
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
}

void ScrobblingManager::timerEvent(QTimerEvent* timerEvent) {
    Q_UNUSED(timerEvent);
    for (TrackInfo* pTrackInfo : m_trackList) {
        bool inaudible = true;
        for (BaseTrackPlayer *player : pTrackInfo->m_players) {
            if (isTrackAudible(pTrackInfo->m_pTrack,player)) {
                inaudible = false;
                break;
            }
        }
        if (inaudible) {
            pTrackInfo->m_trackInfo.pausePlayedTime();
        }
    }
}

void ScrobblingManager::slotReadyToBeScrobbled(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
    qDebug() << "Track ready to be scrobbled";
}
