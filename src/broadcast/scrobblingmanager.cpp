#include <QObject>

#include "broadcast/scrobblingmanager.h"
#include "control/controlproxy.h"
#include "engine/enginexfader.h"
#include "mixer/deck.h"

ScrobblingManager::ScrobblingManager() :
m_CPGuiTick("[Master]", "guiTick50ms",this),
m_CPCrossfader("[Master]","crossfader", this),
m_CPXFaderCurve(ConfigKey(EngineXfader::kXfaderConfigKey, 
                "xFaderCurve"),this),

m_CPXFaderCalibration(ConfigKey(EngineXfader::kXfaderConfigKey, 
                      "xFaderCalibration"),this),

m_CPXFaderMode(ConfigKey(EngineXfader::kXfaderConfigKey, 
               "xFaderMode"),this),

m_CPXFaderReverse(ConfigKey(EngineXfader::kXfaderConfigKey, 
                  "xFaderReverse"),this)
{
    m_CPGuiTick.connectValueChanged(SLOT(slotGuiTick(double)));
    startTimer(1000);
}


void ScrobblingManager::slotTrackPaused(TrackPointer pPausedTrack) {
    //Extra functional decision to only track main decks.            
    Deck *sourceDeck = qobject_cast<Deck*>(sender());    
    if (sourceDeck == 0) {
        qDebug() << "Didn't load track in a deck yet scrobbling "
                    "received paused signal.";
        return;
    }
    // QMutexLocker locker(&m_mutex);
    bool allPaused = true;
    TrackInfo *pausedTrackInfo;
    for (TrackInfo *trackInfo : m_trackList) {
        if (trackInfo->m_pTrack == pPausedTrack) {
            pausedTrackInfo = trackInfo;
            for (BaseTrackPlayer *player : trackInfo->m_players) {
                BaseTrackPlayerImpl *playerImpl = 
                    qobject_cast<BaseTrackPlayerImpl*>(player);
                if (playerImpl == 0) {
                    qDebug() << "Track player interface isn't a "
                                "BaseTrackPlayerImpl";
                    return;
                }
                if (!playerImpl->isTrackPaused())
                    allPaused = false;
            }
            break;
        }                   
    }
    if (allPaused)
        pausedTrackInfo->m_trackInfo.pausePlayedTime();        
}

void ScrobblingManager::slotTrackResumed(TrackPointer pResumedTrack) {
    //Extra functional decision to only track main decks.
    Deck *sourceDeck = qobject_cast<Deck*>(sender());    
    if (sourceDeck == 0) {
        qDebug() << "Didn't load track in a deck yet scrobbling "
                    "received resumed signal.";
        return;
    }    
    if (isTrackAudible(pResumedTrack,sourceDeck)) {        
        // QMutexLocker locker(&m_mutex);
        for (TrackInfo *trackInfo : m_trackList) {
            if (trackInfo->m_pTrack == pResumedTrack) {
                trackInfo->m_trackInfo.resumePlayedTime();
                break;
            }
        }  
    }
}

void ScrobblingManager::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    //Extra functional decision to only track main decks.
    Deck *sourceDeck = qobject_cast<Deck*>(sender());    
    if (sourceDeck == 0) {
        qDebug() << "Didn't load track in a deck yet scrobbling "
                    "received loading signal.";
        return;
    }
    if (pOldTrack) {
        m_tracksToBeReset.append(TrackToBeReset(pOldTrack,sourceDeck));
    }
}

void ScrobblingManager::slotNewTrackLoaded(TrackPointer pNewTrack) {
    //Empty deck gives a null pointer.
    if (!pNewTrack)
        return;    
    //Extra functional decision to only track main decks.        
    Deck *newDeck = qobject_cast<Deck*>(sender());    
    if (newDeck == 0) {
        qDebug() << "Didn't load track in a deck yet scrobbling "
                    "received loaded signal.";
        return;
    }
    // QMutexLocker locker(&m_mutex);    
    bool trackAlreadyAdded = false;
    for (TrackInfo *trackInfo : m_trackList) {
        if (trackInfo->m_pTrack == pNewTrack) {
            trackInfo->m_players.append(newDeck);               
            trackAlreadyAdded = true;
            break;
        }
    }
    if (!trackAlreadyAdded) {
        TrackInfo *newTrackInfo = new TrackInfo(pNewTrack,newDeck);
        newTrackInfo->m_players.append(newDeck);
        m_trackList.append(newTrackInfo);                
        connect(&m_trackList.last()->m_trackInfo,SIGNAL(readyToBeScrobbled(TrackPointer)),
                this,SLOT(slotReadyToBeScrobbled(TrackPointer)));
    }
    //A new track has been loaded so must unload old one.
    resetTracks();
}

void ScrobblingManager::slotPlayerEmpty() {
    // QMutexLocker locker(&m_mutex);
    resetTracks();
}

void ScrobblingManager::resetTracks() {
    for (TrackToBeReset candidateTrack : m_tracksToBeReset) {
        for (TrackInfo *trackInfo : m_trackList) {
            if (trackInfo->m_pTrack == candidateTrack.m_pTrack) {
                if (!trackInfo->m_players.contains(candidateTrack.m_pPlayer)) {
                    qDebug() << "Track doesn't contain player"
                                "yet is requested for deletion.";
                    return;
                }
                //Load error, stray from engine buffer.
                if (candidateTrack.m_pPlayer->getLoadedTrack() ==
                    candidateTrack.m_pTrack) 
                    break;                    
                QLinkedList<BaseTrackPlayer*>::iterator it = 
                    trackInfo->m_players.begin();
                while (it != trackInfo->m_players.end()) {
                    if (*it == candidateTrack.m_pPlayer) {
                        trackInfo->m_players.erase(it);
                    }
                }
                if (trackInfo->m_players.empty()) {
                    trackInfo->m_trackInfo.pausePlayedTime();
                    trackInfo->m_trackInfo.resetPlayedTime();
                    delete trackInfo;
                }
                break;  
            }
        }
    }
}

bool ScrobblingManager::isTrackAudible(TrackPointer pTrack, BaseTrackPlayer * pPlayer) {
    if (pPlayer->getLoadedTrack() != pTrack) {
        qDebug() << "Track can't be audible because is not in player";
        return false;
    }   
    return getPlayerVolume(pPlayer) >= 0.20;
}

double ScrobblingManager::getPlayerVolume(BaseTrackPlayer *pPlayer) {
    double finalVolume;
    ControlProxy trackPreGain(pPlayer->getGroup(),"pregain",this);
    double preGain = trackPreGain.get();
    ControlProxy trackVolume(pPlayer->getGroup(),"volume",this);
    double volume = trackVolume.get();
    ControlProxy deckOrientation(pPlayer->getGroup(),"orientation",this);
    int orientation = deckOrientation.get();    

    double xFaderLeft,xFaderRight;

    EngineXfader::getXfadeGains(m_CPCrossfader.get(),
                                m_CPXFaderCurve.get(),
                                m_CPXFaderCalibration.get(),
                                m_CPXFaderMode.get(),
                                m_CPXFaderReverse.toBool(),
                                &xFaderLeft,&xFaderRight);
    finalVolume = preGain * volume;
    if (orientation == EngineChannel::LEFT)
        finalVolume *= xFaderLeft;
    else if (orientation == EngineChannel::RIGHT) 
        finalVolume *= xFaderRight;
    return finalVolume;
}

void ScrobblingManager::slotGuiTick(double timeSinceLastTick) {
    for (TrackInfo *trackInfo : m_trackList) {
        trackInfo->m_trackInfo.slotGuiTick(timeSinceLastTick);
    }
}

void ScrobblingManager::timerEvent(QTimerEvent *timerEvent) {
    for (TrackInfo *trackInfo : m_trackList) {
        bool inaudible = true;
        for (BaseTrackPlayer *player : trackInfo->m_players) {
            if (isTrackAudible(trackInfo->m_pTrack,player)) {
                inaudible = false;
                break;
            }
        }
        if (inaudible) {
            trackInfo->m_trackInfo.pausePlayedTime();
        }
    }
}

void ScrobblingManager::slotReadyToBeScrobbled(TrackPointer pTrack) {
    qDebug() << "Track ready to be scrobbled";
}