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
#include "engine/enginexfader.h"
#include "playermanager.h"

PlayerInfo::PlayerInfo()
        : m_currentlyPlayingDeck(0) {
    m_iNumDecks = ControlObject::getControl(
        ConfigKey("[Master]","num_decks"))->get();
    for (int i = 0; i < m_iNumDecks; ++i) {
        QString chan = PlayerManager::groupForDeck(i);

        m_listCOPlay[chan] = new ControlObjectThread(
            ControlObject::getControl(ConfigKey(chan, "play")));
        m_listCOVolume[chan] = new ControlObjectThread(
            ControlObject::getControl(ConfigKey(chan, "volume")));
        m_listCOOrientation[chan] = new ControlObjectThread(
            ControlObject::getControl(ConfigKey(chan, "orientation")));
        m_listCOpregain[chan] = new ControlObjectThread(
            ControlObject::getControl(ConfigKey(chan, "pregain")));
    }

    m_iNumSamplers = ControlObject::getControl(
        ConfigKey("[Master]", "num_samplers"))->get();
    for (int i = 0; i < m_iNumSamplers; ++i) {
        QString chan = PlayerManager::groupForSampler(i);

        m_listCOPlay[chan] = new ControlObjectThread(
            ControlObject::getControl(ConfigKey(chan, "play")));
        m_listCOVolume[chan] = new ControlObjectThread(
            ControlObject::getControl(ConfigKey(chan, "volume")));
        m_listCOOrientation[chan] = new ControlObjectThread(
            ControlObject::getControl(ConfigKey(chan, "orientation")));
        m_listCOpregain[chan] = new ControlObjectThread(
            ControlObject::getControl(ConfigKey(chan, "pregain")));
    }

    m_COxfader = new ControlObjectThread(
        ControlObject::getControl(ConfigKey("[Master]","crossfader")));
    startTimer(2000);
}

PlayerInfo::~PlayerInfo() {
    int i;
    m_loadedTrackMap.clear();

    for (i = 1; i <= m_iNumDecks; i++) {
        QString chan = QString("[Channel%1]").arg(i);

        delete m_listCOPlay[chan];
        delete m_listCOVolume[chan];
        delete m_listCOOrientation[chan];
        delete m_listCOpregain[chan];
    }

    delete m_COxfader;
}

PlayerInfo &PlayerInfo::Instance() {
    static PlayerInfo playerInfo;
    return playerInfo;
}

TrackPointer PlayerInfo::getTrackInfo(QString group) {
    QMutexLocker locker(&m_mutex);

    if (m_loadedTrackMap.contains(group)) {
        return m_loadedTrackMap[group];
    }

    return TrackPointer();
}

void PlayerInfo::setTrackInfo(QString group, TrackPointer track)
{
    QMutexLocker locker(&m_mutex);
    TrackPointer pOld = m_loadedTrackMap[group];
    if (pOld) {
        emit(trackUnloaded(group, pOld));
    }
    m_loadedTrackMap[group] = track;
    emit(trackLoaded(group, track));
}

bool PlayerInfo::isTrackLoaded(TrackPointer pTrack) const {
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

bool PlayerInfo::isTrackPlaying(TrackPointer pTrack) const {
    QMutexLocker locker(&m_mutex);
    QMapIterator<QString, TrackPointer> it(m_loadedTrackMap);
    while (it.hasNext()) {
        it.next();
        if (it.value() == pTrack) {
            ControlObjectThread* coPlay = m_listCOPlay[it.key()];
            if (coPlay && coPlay->get() != 0.0) {
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
    int MaxVolume = 0;
    int MaxDeck = 0;
    int i;

    for (i = 1; i <= m_iNumDecks; i++) {
        QString chan = QString("[Channel%1]").arg(i);
        float fvol;
        float xfl, xfr, xfvol;
        float dvol;
        int orient;

        if (m_listCOPlay[chan]->get() == 0.0 )
            continue;

        if (m_listCOpregain[chan]->get() <= 0.5 )
            continue;

        if ((fvol = m_listCOVolume[chan]->get()) == 0.0 )
            continue;

        EngineXfader::getXfadeGains(xfl, xfr, m_COxfader->get(), 1.0, 0.0, false, false);

        // Orientation goes: left is 0, center is 1, right is 2.
        // Leave math out of it...
        orient = m_listCOOrientation[chan]->get();
        if ( orient == 0 )
            xfvol = xfl;
        else if ( orient == 2 )
            xfvol = xfr;
        else
            xfvol = 1;

        dvol = fvol * xfvol;
        if (dvol > MaxVolume ) {
            MaxDeck = i;
            MaxVolume = dvol;
        }
    }

    if (MaxDeck != m_currentlyPlayingDeck) {
        m_currentlyPlayingDeck = MaxDeck;
        m_mutex.unlock();
        emit(currentPlayingDeckChanged(MaxDeck));
    }
}

int PlayerInfo::getCurrentPlayingDeck() {
    QMutexLocker locker(&m_mutex);
    return m_currentlyPlayingDeck;
}

TrackPointer PlayerInfo::getCurrentPlayingTrack() {
    int deck = getCurrentPlayingDeck();
    if (deck) {
        QString chan = QString("[Channel%1]").arg(deck);
        return getTrackInfo(chan);
    }
    return TrackPointer();
}
