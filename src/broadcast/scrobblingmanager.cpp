#include <QObject>

#include "broadcast/scrobblingmanager.h"
#include "broadcast/filelistener.h"
#include "control/controlproxy.h"
#include "engine/enginexfader.h"
#include "mixer/deck.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"

TotalVolumeThreshold::TotalVolumeThreshold(QObject *parent, double threshold) 
        :  m_CPCrossfader("[Master]","crossfader", parent),
           m_CPXFaderCurve(ConfigKey(EngineXfader::kXfaderConfigKey, 
                "xFaderCurve"),parent),
           m_CPXFaderCalibration(ConfigKey(EngineXfader::kXfaderConfigKey, 
                      "xFaderCalibration"),parent),
           m_CPXFaderMode(ConfigKey(EngineXfader::kXfaderConfigKey, 
               "xFaderMode"),parent),
           m_CPXFaderReverse(ConfigKey(EngineXfader::kXfaderConfigKey, 
                  "xFaderReverse"),parent),
           m_pParent(parent),
           m_volumeThreshold(threshold) {

}

bool TotalVolumeThreshold::isTrackAudible(TrackPointer pTrack, 
                                          BaseTrackPlayer *pPlayer) const {
    DEBUG_ASSERT(pPlayer);
    Q_UNUSED(pTrack);
    double finalVolume;
    ControlProxy trackPreGain(pPlayer->getGroup(),"pregain",m_pParent);
    double preGain = trackPreGain.get();
    ControlProxy trackVolume(pPlayer->getGroup(),"volume",m_pParent);
    double volume = trackVolume.get();
    ControlProxy deckOrientation(pPlayer->getGroup(),"orientation",m_pParent);
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
    return finalVolume > m_volumeThreshold;
}

void TotalVolumeThreshold::setVolumeThreshold(double volume) {
    m_volumeThreshold = volume;
}

ScrobblingManager::ScrobblingManager(PlayerManagerInterface *manager) 
        :  m_pManager(manager),
           m_pBroadcaster(new MetadataBroadcaster),    
           m_pAudibleStrategy(new TotalVolumeThreshold(this,0.20)),
           m_pTimer(new TrackTimers::GUITickTimer) {
    connect(&PlayerInfo::instance(),SIGNAL(currentPlayingTrackChanged(TrackPointer)),
            m_pBroadcaster.get(),SLOT(slotNowListening(TrackPointer)));
    connect(m_pTimer.get(),SIGNAL(timeout()),
            this,SLOT(slotCheckAudibleTracks()));
    m_pTimer->start(1000);
    m_pBroadcaster
        ->addNewScrobblingService(
          FileListener::makeFileListener(
              FileListener::FileListenerType::SAMBroadcaster,
              "nowListening.txt"));    
}

ScrobblingManager::~ScrobblingManager() {
    for (TrackInfo *info : m_trackList) {
        delete info;
    }
}

void ScrobblingManager::setAudibleStrategy(TrackAudibleStrategy *pStrategy) {
    m_pAudibleStrategy.reset(pStrategy);
}

void ScrobblingManager::setMetadataBroadcaster(MetadataBroadcasterInterface *pBroadcast) {
    m_pBroadcaster.reset(pBroadcast);
}

void ScrobblingManager::setTimer(TrackTimers::RegularTimer *timer) {
    m_pTimer.reset(timer);
}

void ScrobblingManager::setTrackInfoFactory(
    const std::function<std::shared_ptr<TrackTimingInfo>(TrackPointer)> &factory) {
    m_trackInfoFactory = factory;
}


void ScrobblingManager::slotTrackPaused(TrackPointer pPausedTrack) {
    bool allPaused = true;
    TrackInfo *pausedTrackInfo = nullptr;
    for (TrackInfo *trackInfo : m_trackList) {
        DEBUG_ASSERT(trackInfo);
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
        pausedTrackInfo->m_trackInfo->pausePlayedTime();        
}

void ScrobblingManager::slotTrackResumed(TrackPointer pResumedTrack) {
    BaseTrackPlayer *player = qobject_cast<Deck*>(sender());
    DEBUG_ASSERT(player);       
    if (m_pAudibleStrategy->isTrackAudible(pResumedTrack,player)) {       
        for (TrackInfo *trackInfo : m_trackList) {
            DEBUG_ASSERT(trackInfo);
            if (trackInfo->m_pTrack == pResumedTrack && 
                trackInfo->m_trackInfo->isTimerPaused()) {
                trackInfo->m_trackInfo->resumePlayedTime();
                break;
            }
        }  
    }
}

void ScrobblingManager::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pNewTrack);
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
    DEBUG_ASSERT(player);    
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
        if (m_trackInfoFactory) {
            newTrackInfo->m_trackInfo = m_trackInfoFactory(pNewTrack);
        }
        m_trackList.append(newTrackInfo);                
        connect(m_trackList.last()->m_trackInfo.get(),SIGNAL(readyToBeScrobbled(TrackPointer)),
                this,SLOT(slotReadyToBeScrobbled(TrackPointer)));
        m_pBroadcaster->newTrackLoaded(pNewTrack);
    }
    //A new track has been loaded so must unload old one.
    resetTracks();
}

void ScrobblingManager::slotPlayerEmpty() {
    resetTracks();
}

void ScrobblingManager::resetTracks() {
    for (TrackToBeReset candidateTrack : m_tracksToBeReset) {        
        for (auto it = m_trackList.begin();
             it != m_trackList.end(); 
             ++it) {
            TrackInfo *trackInfo = *it;
            if (trackInfo->m_pTrack == candidateTrack.m_pTrack) {                                 
                if (playerNotInTrackList(trackInfo->m_players,
                                         candidateTrack.m_playerGroup) ||
                    isStrayFromEngine(trackInfo->m_pTrack,
                                      candidateTrack.m_playerGroup)) {
                    break;                      
                }
                deletePlayerFromList(candidateTrack.m_playerGroup,
                                     trackInfo->m_players);
                if (trackInfo->m_players.empty()) {
                    deleteTrackInfoAndNotify(it);
                }                 
            }
        }
    }
}

bool ScrobblingManager::isStrayFromEngine(TrackPointer pTrack,
                                          const QString &group) const {
    BaseTrackPlayer *player = m_pManager->getPlayer(group);
    return player->getLoadedTrack() == pTrack;
}

bool ScrobblingManager::playerNotInTrackList(const QLinkedList<QString> &list, 
                                             const QString &group) const {
    qDebug() << "Player added to reset yet not in track list";
    return list.contains(group);
}

void ScrobblingManager::deletePlayerFromList(const QString &player,
                                             QLinkedList<QString> &list) {
    QLinkedList<QString>::iterator it;
    for (it = list.begin(); 
         it != list.end() && *it != player; 
         ++it);
    if (*it == player) {
        list.erase(it);
    }
}

void ScrobblingManager::deleteTrackInfoAndNotify(QLinkedList<TrackInfo*>::iterator &it) {
    (*it)->m_trackInfo->pausePlayedTime();
    (*it)->m_trackInfo->resetPlayedTime();
    m_pBroadcaster->trackUnloaded((*it)->m_pTrack);                
    delete *it;
    m_trackList.erase(it);
}



void ScrobblingManager::slotGuiTick(double timeSinceLastTick) {
    for (TrackInfo *trackInfo : m_trackList) {
        trackInfo->m_trackInfo->slotGuiTick(timeSinceLastTick);
    }

    MetadataBroadcaster *broadcaster =
        qobject_cast<MetadataBroadcaster*>(m_pBroadcaster.get());
    if (broadcaster)
        broadcaster->guiTick(timeSinceLastTick);

    TrackTimers::GUITickTimer *timer = 
        qobject_cast<TrackTimers::GUITickTimer*>(m_pTimer.get());
    if (timer)
        timer->slotTick(timeSinceLastTick);    
}

void ScrobblingManager::slotReadyToBeScrobbled(TrackPointer pTrack) {
    m_pBroadcaster->slotAttemptScrobble(pTrack);
}

void ScrobblingManager::slotCheckAudibleTracks() {
    for (TrackInfo *trackInfo : m_trackList) {
        bool inaudible = true;
        for (QString playerGroup : trackInfo->m_players) {
            BaseTrackPlayer *player = m_pManager->getPlayer(playerGroup);
            if (m_pAudibleStrategy->isTrackAudible(trackInfo->m_pTrack,player)) {
                inaudible = false;
                break;
            }
        }
        if (inaudible) {
            trackInfo->m_trackInfo->pausePlayedTime();
        }
        else if (trackInfo->m_trackInfo->isTimerPaused()){
            trackInfo->m_trackInfo->resumePlayedTime();
        }
    }
    m_pTimer->start(1000);
}