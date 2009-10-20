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

#include "playerinfo.h"
#include "trackinfoobject.h"
#include <QtDebug>

PlayerInfo &PlayerInfo::Instance()
{
    static PlayerInfo playerInfo;
    return playerInfo;
}

PlayerInfo::PlayerInfo()
        : m_pTrack1(NULL),
          m_pTrack2(NULL) {
}

PlayerInfo::~PlayerInfo()
{
}

TrackInfoObject *PlayerInfo::getTrackInfo(int track)
{
    TrackInfoObject* pRet = NULL;
    m_mutex.lock();
    switch (track)
    {
    case 1:
        pRet = m_pTrack1;
        break;
    case 2:
        pRet = m_pTrack2;
        break;
    default:
        // incorrect track number
        pRet = NULL;
    }
    m_mutex.unlock();
    return pRet;
}

void PlayerInfo::setTrackInfo(int track, TrackInfoObject *trackInfoObj)
{
    m_mutex.lock();
    switch (track) {
        case 1:
            m_pTrack1 = trackInfoObj;
            break;
        case 2:
            m_pTrack2 = trackInfoObj;
            break;
    };
    m_mutex.unlock();
}
