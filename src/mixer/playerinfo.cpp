/***************************************************************************
                      playerinfo.cpp  -  Helper class to have easy access
                                         to a lot of data (singleton)
                             -------------------
    copyright            : (C) 2007 by Wesley Stessens
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mixer/playerinfo.h"

#include <QMutexLocker>

#include "control/controlobject.h"
#include "engine/channels/enginechannel.h"
#include "engine/enginexfader.h"
#include "mixer/playermanager.h"
#include "track/track.h"

namespace {

const int kPlayingDeckUpdateIntervalMillis = 2000;

PlayerInfo* s_pPlayerInfo = nullptr;

}

PlayerInfo::PlayerInfo()
        : m_pCOxfader(new ControlProxy("[Master]","crossfader", this)),
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
    QMutexLocker locker(&m_mutex);
    return m_loadedTrackMap.value(group);
}

void PlayerInfo::setTrackInfo(const QString& group, const TrackPointer& track) {
    TrackPointer pOld;
    { // Scope
        QMutexLocker locker(&m_mutex);
        pOld = m_loadedTrackMap.value(group);
        m_loadedTrackMap.insert(group, track);
    }
    if (pOld) {
        emit trackUnloaded(group, pOld);
    }
    emit trackLoaded(group, track);
}

bool PlayerInfo::isTrackLoaded(const TrackPointer& pTrack) const {
    QMutexLocker locker(&m_mutex);
    QMapIterator<QString, TrackPointer> it(m_loadedTrackMap);
    while (it.hasNext()) {
        it.next();
        if (it.value() == pTrack) {
            return true;
        }
    }
    return false;
}

QMap<QString, TrackPointer> PlayerInfo::getLoadedTracks() {
    QMutexLocker locker(&m_mutex);
    QMap<QString, TrackPointer> ret = m_loadedTrackMap;
    return ret;
}

bool PlayerInfo::isFileLoaded(const QString& track_location) const {
    QMutexLocker locker(&m_mutex);
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
    QMutexLocker locker(&m_mutex);

    double maxVolume = 0;
    int maxDeck = -1;

    for (int i = 0; i < (int)PlayerManager::numDecks(); ++i) {
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

        CSAMPLE_GAIN xfl, xfr;
        // TODO: supply correct parameters to the function. If the hamster style
        // for the crossfader is enabled, the result is currently wrong.
        EngineXfader::getXfadeGains(m_pCOxfader->get(), 1.0, 0.0, MIXXX_XFADER_ADDITIVE, false,
                                    &xfl, &xfr);

        int orient = pDc->m_orientation.get();
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
    if (maxDeck != m_currentlyPlayingDeck) {
        m_currentlyPlayingDeck = maxDeck;
        locker.unlock();
        emit currentPlayingDeckChanged(maxDeck);
        emit currentPlayingTrackChanged(getCurrentPlayingTrack());
    }
}

int PlayerInfo::getCurrentPlayingDeck() {
    QMutexLocker locker(&m_mutex);
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
