#include <QObject>

#include "broadcast/scrobblingmanager.h"
#include "broadcast/filelistener.h"
#include "control/controlproxy.h"
#include "engine/enginexfader.h"
#include "mixer/deck.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"

ScrobblingManager::ScrobblingManager(PlayerManager *manager) 
        :  m_CPGuiTick("[Master]", "guiTick50ms",this),
           m_CPCrossfader("[Master]","crossfader", this),
           m_CPXFaderCurve(ConfigKey(EngineXfader::kXfaderConfigKey, 
                "xFaderCurve"),this),
           m_CPXFaderCalibration(ConfigKey(EngineXfader::kXfaderConfigKey, 
                      "xFaderCalibration"),this),
           m_CPXFaderMode(ConfigKey(EngineXfader::kXfaderConfigKey, 
               "xFaderMode"),this),
           m_CPXFaderReverse(ConfigKey(EngineXfader::kXfaderConfigKey, 
                  "xFaderReverse"),this),
           m_pManager(manager),
           m_broadcaster() {
    m_CPGuiTick.connectValueChanged(SLOT(slotGuiTick(double)));
    connect(&PlayerInfo::instance(),SIGNAL(currentPlayingTrackChanged(TrackPointer)),
            &m_broadcaster,SLOT(slotNowListening(TrackPointer)));
    m_broadcaster
        .addNewScrobblingService(new FileListener("nowListening.txt"));
    startTimer(1000);
}


void ScrobblingManager::slotTrackPaused(TrackPointer pPausedTrack) {         
    QMutexLocker locker(&m_mutex);
    bool allPaused = true;
    TrackInfo *pausedTrackInfo = nullptr;
    for (TrackInfo *trackInfo : m_trackList) {
        VERIFY_OR_DEBUG_ASSERT(trackInfo);
        if (trackInfo->m_pTrack == pPausedTrack) {
            pausedTrackInfo = trackInfo;
            for (QString playerGroup : trackInfo->m_players) {
                BaseTrackPlayer *player = m_pManager->getPlayer(playerGroup);
                if (!player->isTrackPaused())
                    allPaused = false;
            }
            break;
        }                   
    }
    if (allPaused && pausedTrackInfo)
        pausedTrackInfo->m_trackInfo.pausePlayedTime();        
}

void ScrobblingManager::slotTrackResumed(TrackPointer pResumedTrack) {
    BaseTrackPlayer *player = qobject_cast<Deck*>(sender());
    VERIFY_OR_DEBUG_ASSERT(player);       
    if (isTrackAudible(pResumedTrack,player)) {        
        QMutexLocker locker(&m_mutex);        
        for (TrackInfo *trackInfo : m_trackList) {
            VERIFY_OR_DEBUG_ASSERT(trackInfo);
            if (trackInfo->m_pTrack == pResumedTrack && 
                trackInfo->m_trackInfo.isTimerPaused()) {
                trackInfo->m_trackInfo.resumePlayedTime();
                break;
            }
        }  
    }
}

void ScrobblingManager::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    BaseTrackPlayer *sourcePlayer = 
        qobject_cast<BaseTrackPlayer*>(sender());
    DEBUG_ASSERT(sourcePlayer);
    if (pOldTrack) {
        m_tracksToBeReset.append(TrackToBeReset(pOldTrack,
            sourcePlayer->getGroup()));
    }
}

void ScrobblingManager::slotNewTrackLoaded(TrackPointer pNewTrack) {
    //Empty player gives a null pointer.
    if (!pNewTrack)
        return;          
    BaseTrackPlayer *player = qobject_cast<BaseTrackPlayer*>(sender());    
    VERIFY_OR_DEBUG_ASSERT(player);
    QMutexLocker locker(&m_mutex);    
    bool trackAlreadyAdded = false;
    for (TrackInfo *trackInfo : m_trackList) {        
        if (trackInfo->m_pTrack == pNewTrack) {
            trackInfo->m_players.append(player->getGroup());               
            trackAlreadyAdded = true;
            break;
        }
    }
    if (!trackAlreadyAdded) {
        TrackInfo *newTrackInfo = new TrackInfo(pNewTrack);
        newTrackInfo->m_players.append(player->getGroup());
        m_trackList.append(newTrackInfo);                
        connect(&m_trackList.last()->m_trackInfo,SIGNAL(readyToBeScrobbled(TrackPointer)),
                &m_broadcaster,SLOT(slotAttemptScrobble(TrackPointer)));
        m_broadcaster.newTrackLoaded(pNewTrack);
    }
    //A new track has been loaded so must unload old one.
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
        for (;trackListIterator != m_trackList.end(); ++trackListIterator) {
            TrackInfo *trackInfo = *trackListIterator;
            if (trackInfo->m_pTrack == candidateTrack.m_pTrack) {
                if (!trackInfo->m_players.contains(candidateTrack.
                    m_playerGroup)) {
                    qDebug() << "Track doesn't contain player"
                                "yet is requested for deletion.";
                    break;
                }
                //Load error, stray from engine buffer.
                BaseTrackPlayer *player = 
                    m_pManager->getPlayer(candidateTrack.m_playerGroup);
                if (player->getLoadedTrack() ==
                    candidateTrack.m_pTrack) 
                    break;
                //Delete player from player list.                    
                QLinkedList<QString>::iterator it = 
                    trackInfo->m_players.begin();
                while (it != trackInfo->m_players.end() &&
                       *it != candidateTrack.m_playerGroup) {                    
                    ++it;
                }
                if (*it == candidateTrack.m_playerGroup) {
                    trackInfo->m_players.erase(it);
                }
                //If player list is empty, notify and erase.
                if (trackInfo->m_players.empty()) {
                    trackInfo->m_trackInfo.pausePlayedTime();
                    trackInfo->m_trackInfo.resetPlayedTime();
                    delete trackInfo;
                    m_trackList.erase(trackListIterator);
                    m_broadcaster.trackUnloaded(trackInfo->m_pTrack);
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
    m_broadcaster.slotGuiTick(timeSinceLastTick);
}

void ScrobblingManager::timerEvent(QTimerEvent *timerEvent) {
    for (TrackInfo *trackInfo : m_trackList) {
        bool inaudible = true;
        for (QString playerGroup : trackInfo->m_players) {
            BaseTrackPlayer *player = m_pManager->getPlayer(playerGroup);
            if (isTrackAudible(trackInfo->m_pTrack,player)) {
                inaudible = false;
                break;
            }
        }
        if (inaudible) {
            trackInfo->m_trackInfo.pausePlayedTime();
        }
        else if (trackInfo->m_trackInfo.isTimerPaused()){
            trackInfo->m_trackInfo.resumePlayedTime();
        }
    }
}