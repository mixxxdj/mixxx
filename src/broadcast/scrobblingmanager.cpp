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
    Deck* sourceDeck = qobject_cast<Deck*>(sender());
    if (sourceDeck == 0) {
        qDebug() << "Didn't load track in a deck yet scrobbling "
                    "received paused signal.";
        return;
    }
    QMutexLocker locker(&m_mutex);
    QLinkedList<TrackInfo*>::Iterator it = m_trackList.begin();
    while (it != m_trackList.end()) {
        if ((*it)->m_pTrack == pPausedTrack and (*it)->m_pPlayer == sourceDeck) {
            (*it)->m_trackInfo.pausePlayedTime();
            break;
        }
        ++it;
    }
}

void ScrobblingManager::slotTrackResumed(TrackPointer pPausedTrack) {
    Deck* sourceDeck = qobject_cast<Deck*>(sender());
    if (sourceDeck == 0) {
        qDebug() << "Didn't load track in a deck yet scrobbling "
                    "received resumed signal.";
        return;
    }
    if (isTrackAudible(pPausedTrack, sourceDeck)) {
        QMutexLocker locker(&m_mutex);
        QLinkedList<TrackInfo*>::Iterator it = m_trackList.begin();
        while (it != m_trackList.end()) {
            if ((*it)->m_pTrack == pPausedTrack and (*it)->m_pPlayer == sourceDeck) {
                (*it)->m_trackInfo.pausePlayedTime();
                break;
            }
            ++it;
        }
    }
}

void ScrobblingManager::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pNewTrack);
    Deck* sourceDeck = qobject_cast<Deck*>(sender());
    if (sourceDeck == 0) {
        qDebug() << "Didn't load track in a deck yet scrobbling "
                    "received loading signal.";
        return;
    }
    if (pOldTrack) {
        m_tracksToBeReset.append(TrackToBeReset(pOldTrack, sourceDeck));
    }
}

void ScrobblingManager::slotNewTrackLoaded(TrackPointer pNewTrack) {
    if (not pNewTrack)
        return;
    Deck* newDeck = qobject_cast<Deck*>(sender());
    if (newDeck == 0) {
        qDebug() << "Didn't load track in a deck yet scrobbling "
                    "received loaded signal.";
        return;
    }
    QMutexLocker locker(&m_mutex);
    m_trackList.append(new TrackInfo(pNewTrack, newDeck));
    connect(&m_trackList.last()->m_trackInfo, SIGNAL(readyToBeScrobbled(TrackPointer)), this, SLOT(slotReadyToBeScrobbled(TrackPointer)));
    resetTracks();
}

void ScrobblingManager::slotPlayerEmpty() {
    QMutexLocker locker(&m_mutex);
    resetTracks();
}

void ScrobblingManager::resetTracks() {
    QLinkedList<TrackToBeReset>::Iterator itReset = m_tracksToBeReset.begin();
    while (itReset != m_tracksToBeReset.end()) {
        QLinkedList<TrackInfo*>::Iterator itListening = m_trackList.begin();
        while (itListening != m_trackList.end()) {
            if ((*itListening)->m_pTrack == itReset->m_pTrack and
                    (*itListening)->m_pPlayer == itReset->m_pPlayer) {
                delete (*itListening);
                m_trackList.erase(itListening);
                break;
            }
            ++itListening;
        }
        ++itReset;
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
    foreach (TrackInfo* trackInfo, m_trackList) {
        trackInfo->m_trackInfo.slotGuiTick(timeSinceLastTick);
    }
}

void ScrobblingManager::timerEvent(QTimerEvent* timerEvent) {
    Q_UNUSED(timerEvent);
    foreach (TrackInfo* trackInfo, m_trackList) {
        if (not isTrackAudible(trackInfo->m_pTrack, trackInfo->m_pPlayer)) {
            trackInfo->m_trackInfo.pausePlayedTime();
        }
    }
}

void ScrobblingManager::slotReadyToBeScrobbled(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
    qDebug() << "Track ready to be scrobbled";
}
