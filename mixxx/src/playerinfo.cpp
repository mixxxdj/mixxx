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
{
}

PlayerInfo::~PlayerInfo()
{
}

TrackInfoObject *PlayerInfo::getTrackInfo(int track) const
{
    switch (track)
    {
    case 1:
        return m_pTrack1;
        break; // not really needed
    case 2:
        return m_pTrack2;
        break; // not really needed
    default:
        // incorrect track number
        return 0;
    }
}

void PlayerInfo::setTrackInfo(int track, TrackInfoObject *trackInfoObj)
{
    if (track==1||track==2) emit trackInfoChanged(track, trackInfoObj);
    switch (track)
    {
    case 1:
        m_pTrack1 = trackInfoObj;
        break;
    case 2:
        m_pTrack2 = trackInfoObj;
        break;
    };
}
