// Helper class to have easy access
#pragma once

#include <QAtomicInt>
#include <QMap>
#include <QMutex>
#include <QObject>

#include "control/pollingcontrolproxy.h"
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
    QStringList getPlayerGroupsWithTracksLoaded(const TrackPointerList& tracks) const;
    bool isTrackLoaded(const TrackPointer& pTrack) const;
    bool isFileLoaded(const QString& track_location) const;

    int numDecks() const;
    int numPreviewDecks() const;
    int numSamplers() const;

  signals:
    void currentPlayingDeckChanged(int deck);
    void currentPlayingTrackChanged(TrackPointer pTrack);
    void trackChanged(const QString& group, TrackPointer pNewTrack, TrackPointer pOldTrack);

  private:
    class DeckControls {
        public:
          DeckControls(const QString& group)
                  : m_play(group, "play"),
                    m_pregain(group, "pregain"),
                    m_volume(group, "volume"),
                    m_orientation(group, "orientation") {
          }

          PollingControlProxy m_play;
          PollingControlProxy m_pregain;
          PollingControlProxy m_volume;
          PollingControlProxy m_orientation;
    };

    void clearControlCache();
    void timerEvent(QTimerEvent* pTimerEvent) override;
    void updateCurrentPlayingDeck();
    DeckControls* getDeckControls(int i);

    PlayerInfo();
    ~PlayerInfo() override;

    mutable QMutex m_mutex;

    PollingControlProxy m_xfader;
    PollingControlProxy m_numDecks;
    PollingControlProxy m_numSamplers;
    PollingControlProxy m_numPreviewDecks;

    // QMap is faster than QHash for small count of elements < 50
    QMap<QString, TrackPointer> m_loadedTrackMap;
    QAtomicInt m_currentlyPlayingDeck;
    QList<DeckControls*> m_deckControlList;

    static PlayerInfo* m_pPlayerinfo;
};
