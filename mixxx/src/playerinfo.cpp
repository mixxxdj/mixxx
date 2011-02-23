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

PlayerInfo &PlayerInfo::Instance()
{
    static PlayerInfo playerInfo;
    return playerInfo;
}

PlayerInfo::PlayerInfo() {
}

PlayerInfo::~PlayerInfo()
{
    m_loadedTrackMap.clear();
}

TrackPointer PlayerInfo::getTrackInfo(QString group)
{
    QMutexLocker locker(&m_mutex);

    if (m_loadedTrackMap.contains(group)) {
        return m_loadedTrackMap[group];
    }

    return TrackPointer();
}

void PlayerInfo::setTrackInfo(QString group, TrackPointer track)
{
    QMutexLocker locker(&m_mutex);
    m_loadedTrackMap[group] = track;
}
