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

#ifndef MIXER_PLAYERINFO_H
#define MIXER_PLAYERINFO_H

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QTimerEvent>

#include "control/controlproxy.h"
#include "track/track_decl.h"

class PlayerInfo : public QObject {
    Q_OBJECT
  public:
    static PlayerInfo& create();
    static PlayerInfo& instance();
    static void destroy();
    TrackPointer getTrackInfo(const QString& group);
    void setTrackInfo(const QString& group, const TrackPointer& trackInfoObj);
    TrackPointer getCurrentPlayingTrack();
    int getCurrentPlayingDeck();
    QMap<QString, TrackPointer> getLoadedTracks();
    bool isTrackLoaded(const TrackPointer& pTrack) const;
    bool isFileLoaded(const QString& track_location) const;

  signals:
    void currentPlayingDeckChanged(int deck);
    void currentPlayingTrackChanged(TrackPointer pTrack);
    void trackLoaded(QString group, TrackPointer pTrack);
    void trackUnloaded(QString group, TrackPointer pTrack);

  private:
    class DeckControls {
        public:
            DeckControls(QString& group)
                    : m_play(group, "play"),
                      m_pregain(group, "pregain"),
                      m_volume(group, "volume"),
                      m_orientation(group, "orientation") {
            }

            ControlProxy m_play;
            ControlProxy m_pregain;
            ControlProxy m_volume;
            ControlProxy m_orientation;
    };

    void clearControlCache();
    void timerEvent(QTimerEvent* pTimerEvent) override;
    void updateCurrentPlayingDeck();
    DeckControls* getDeckControls(int i);

    PlayerInfo();
    ~PlayerInfo() override;

    mutable QMutex m_mutex;
    ControlProxy* m_pCOxfader;
    // QMap is faster than QHash for small count of elements < 50
    QMap<QString, TrackPointer> m_loadedTrackMap;
    int m_currentlyPlayingDeck;
    QList<DeckControls*> m_deckControlList;

    static PlayerInfo* m_pPlayerinfo;
};

#endif /* MIXER_PLAYERINFO_H */
