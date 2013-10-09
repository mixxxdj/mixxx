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

#include <QMutexLocker>

#include "playerinfo.h"
#include "controlobject.h"
#include "controlobjectthread.h"
#include "engine/enginechannel.h"
#include "engine/enginexfader.h"
#include "playermanager.h"

static const int kPlayingDeckUpdateIntervalMillis = 2000;

PlayerInfo::PlayerInfo()
        : m_COxfader("[Master]","crossfader"),
          m_currentlyPlayingDeck(-1) {
    startTimer(kPlayingDeckUpdateIntervalMillis);
}

PlayerInfo::~PlayerInfo() {
    m_loadedTrackMap.clear();
}

PlayerInfo &PlayerInfo::Instance() {
    static PlayerInfo playerInfo;
    return playerInfo;
}

TrackPointer PlayerInfo::getTrackInfo(const QString& group) {
    QMutexLocker locker(&m_mutex);
    return m_loadedTrackMap.value(group);
}

void PlayerInfo::setTrackInfo(const QString& group, const TrackPointer& track) {
    QMutexLocker locker(&m_mutex);
    TrackPointer pOld = m_loadedTrackMap.value(group);
    if (pOld) {
        emit(trackUnloaded(group, pOld));
    }
    m_loadedTrackMap.insert(group, track);
    emit(trackLoaded(group, track));
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

    for (unsigned int i = 0; i < PlayerManager::numDecks(); ++i) {
        QString group = PlayerManager::groupForDeck(i);

        if (ControlObject::get(ConfigKey(group, "play")) == 0.0) {
            continue;
        }

        if (ControlObject::get(ConfigKey(group, "pregain")) <= 0.5) {
            continue;
        }

        double fvol = ControlObject::get(ConfigKey(group, "volume"));
        if (fvol == 0.0) {
            continue;
        }

        double xfl, xfr;
        EngineXfader::getXfadeGains(m_COxfader.get(), 1.0, 0.0, false, false,
                                    &xfl, &xfr);

        int orient = ControlObject::get(ConfigKey(group, "orientation"));
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
        emit(currentPlayingDeckChanged(maxDeck));
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
