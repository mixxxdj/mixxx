// Helper class to have easy access
#pragma once

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
    void trackLoaded(const QString& group, TrackPointer pTrack);
    void trackUnloaded(const QString& group, TrackPointer pTrack);

  private:
    class DeckControls {
        public:
          DeckControls(const QString& group)
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
