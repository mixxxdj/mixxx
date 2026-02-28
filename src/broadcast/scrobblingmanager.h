#include <memory>

#include "util/parented_ptr.h"

#pragma once

#include <QHash>
#include <QLinkedList>
#include <QObject>
#include <QSet>
#include <QString>
#include <functional>

#include "broadcast/metadatabroadcast.h"
#include "control/controlobject.h"
#include "track/track.h"
#include "track/trackplaytimers.h"
#include "track/tracktiminginfo.h"

class BaseTrackPlayer;
class PlayerManager;
class PlayerManagerInterface;
class MixxxMainWindow;

/// Interface for helper class to find audible players.
class TrackAudibleStrategy {
  public:
    virtual ~TrackAudibleStrategy() = default;
    virtual bool isPlayerAudible(BaseTrackPlayer* pPlayer) const = 0;
};

/// Helper class to find audible players
class TotalVolumeThreshold : public TrackAudibleStrategy {
  public:
    TotalVolumeThreshold(QObject* parent, double threshold);
    bool isPlayerAudible(BaseTrackPlayer* pPlayer) const override;
    void setVolumeThreshold(double volume);

  private:
    ControlProxy m_CPCrossfader;
    ControlProxy m_CPXFaderCurve;
    ControlProxy m_CPXFaderCalibration;
    ControlProxy m_CPXFaderMode;
    ControlProxy m_CPXFaderReverse;

    double m_volumeThreshold;
};

typedef std::function<std::shared_ptr<TrackTimingInfo>(TrackPointer)> TrackTimingFactory;

/// Manages all metadata scrobbling services.
class ScrobblingManager : public QObject {
    Q_OBJECT
  public:
    ScrobblingManager(UserSettingsPointer pConfig,
            std::shared_ptr<PlayerManager> pPlayerManager);
    ~ScrobblingManager() = default;
    // Take ownership of the timer
    void setAudibleStrategy(TrackAudibleStrategy* pStrategy);
    void setTimer(parented_ptr<TrackTimers::RegularTimer> timer);
    void setTrackInfoFactory(
            const std::function<std::shared_ptr<TrackTimingInfo>(TrackPointer)>&
                    factory);
    bool hasScrobbledAnyTrack() const;

    template<class T, typename... Args>
    void addScrobblingService(Args&&... args);
    template<class T>
    void removeScrobblingService() {
        m_pBroadcaster->removeScrobblingService<T>();
    }
    template<class T>
    bool isScrobblingServiceActivated() {
        return m_pBroadcaster->isScrobblingServiceActivated<T>();
    }

  public slots:
    void slotTrackPaused(TrackPointer pPausedTrack);
    void slotTrackResumed(TrackPointer pResumedTrack, const QString& playerGroup);
    void slotNewTrackLoaded(TrackPointer pNewTrack, const QString& playerGroup);
    void onNumberOfDecksChanged(int i);

  private:
    struct TrackInfo {
        std::shared_ptr<TrackTimingInfo> m_trackInfo;
        QSet<QString> m_players;
        void init(const TrackTimingFactory& factory,
                const TrackPointer& pTrack) {
            if (factory) {
                m_trackInfo = factory(pTrack);
            } else {
                m_trackInfo = std::make_shared<TrackTimingInfo>(pTrack);
            }
        }
    };

    QHash<TrackId, TrackInfo> m_trackInfoHashDict;

    std::shared_ptr<PlayerManager> m_pPlayerManager;
    UserSettingsPointer m_pConfig;
    std::unique_ptr<MetadataBroadcaster> m_pBroadcaster;
    std::unique_ptr<TrackAudibleStrategy> m_pAudibleStrategy;
    parented_ptr<TrackTimers::RegularTimer> m_pTimer;

    TrackTimingFactory m_trackInfoFactory;

    bool m_scrobbledAtLeastOnce;

    ControlProxy m_GuiTickObject;

    QList<QMetaObject::Connection> m_deckConnections;

  private slots:
    void slotReadyToBeScrobbled(TrackPointer pTrack);
    void slotCheckAudibleTracks();
    void slotGuiTick(double timeSinceLastTick);
};
