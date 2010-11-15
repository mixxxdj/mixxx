/***************************************************************************
                        playerinfo.h  -  Helper class to have easy access
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

#ifndef PLAYERINFO_H
#define PLAYERINFO_H

#include <QObject>
#include <QMutex>
#include <QMap>

#include "trackinfoobject.h"

class PlayerInfo : public QObject
{
    Q_OBJECT
  public:
    static PlayerInfo &Instance();
    TrackPointer getTrackInfo(QString group);
    void setTrackInfo(QString group, TrackPointer trackInfoObj);
  private:
    PlayerInfo();
    ~PlayerInfo();
    PlayerInfo(PlayerInfo const&);
    PlayerInfo &operator= (PlayerInfo const&);
    QMutex m_mutex;
    QMap<QString, TrackPointer> m_loadedTrackMap;
};

#endif
