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
#include <QTimerEvent>

class ControlObjectThread;

#include "trackinfoobject.h"

class PlayerInfo : public QObject
{
    Q_OBJECT
  public:
    static PlayerInfo &Instance();
    TrackPointer getTrackInfo(QString group);
    void setTrackInfo(QString group, TrackPointer trackInfoObj);
    int getCurrentPlayingDeck();
    TrackPointer getCurrentPlayingTrack();
    bool isTrackLoaded(TrackPointer pTrack) const;
    bool isTrackPlaying(TrackPointer pTrack) const;

  signals:
    void currentPlayingDeckChanged(int deck);
    void trackLoaded(QString group, TrackPointer pTrack);
    void trackUnloaded(QString group, TrackPointer pTrack);

  private:
    void timerEvent(QTimerEvent* pTimerEvent);
    void updateCurrentPlayingDeck();

    PlayerInfo();
    ~PlayerInfo();
    PlayerInfo(PlayerInfo const&);
    PlayerInfo &operator= (PlayerInfo const&);
    mutable QMutex m_mutex;
    int m_iNumDecks;
    int m_iNumSamplers;
    ControlObjectThread* m_COxfader;
    QMap<QString, TrackPointer> m_loadedTrackMap;
    QMap<QString, ControlObjectThread*> m_listCOPlay;
    QMap<QString, ControlObjectThread*> m_listCOVolume;
    QMap<QString, ControlObjectThread*> m_listCOOrientation;
    QMap<QString, ControlObjectThread*> m_listCOpregain;

    int m_currentlyPlayingDeck;
};

#endif
