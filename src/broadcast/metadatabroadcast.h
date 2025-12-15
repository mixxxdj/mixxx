#pragma once

#include <QList>
#include <QObject>
#include <list>

#include "broadcast/scrobblingservice.h"
#include "track/track_decl.h"
#include "track/trackid.h"
#include "track/trackplaytimers.h"

/// Interface for MetadataBoadcaster
class MetadataBroadcasterInterface : public QObject {
    Q_OBJECT
  public slots:
    virtual void slotNowListening(TrackPointer pTrack) = 0;
    virtual void slotAttemptScrobble(TrackPointer pTrack) = 0;
    virtual void slotAllTracksPaused() = 0;

  public:
    virtual void newTrackLoaded(TrackPointer pTrack) = 0;
    virtual void trackUnloaded(TrackPointer pTrack) = 0;
};

/// Listen to metadata changes of the current track and passes it to
/// the different scrobbling services.
class MetadataBroadcaster : public MetadataBroadcasterInterface {
    Q_OBJECT
  private:
    struct GracePeriod {
        double m_msSinceEjection;
        unsigned int m_timesTrackHasBeenScrobbled = 0;
        bool m_firstTimeLoaded = true;
        bool m_hasBeenEjected = false;
        GracePeriod()
                : m_msSinceEjection(0.0) {
        }
    };

  public:
    MetadataBroadcaster();

    template<class T, typename... Args>
    void addScrobblingService(Args&&... args) {
        static_assert(std::is_base_of<ScrobblingService, T>::value,
                "Service is not derived from ScrobblingService");
        VERIFY_OR_DEBUG_ASSERT(!m_scrobblingServices.contains(typeid(T).hash_code())) {
            return;
        };
        m_scrobblingServices.emplace(typeid(T).hash_code(), std::make_shared<T>(args...));
    }
    template<class T>
    void removeScrobblingService() {
        static_assert(std::is_base_of<ScrobblingService, T>::value,
                "Service is not derived from ScrobblingService");
        m_scrobblingServices.remove(typeid(T).hash_code());
    }
    template<class T>
    bool isScrobblingServiceActivated() {
        static_assert(std::is_base_of<ScrobblingService, T>::value,
                "Service is not derived from ScrobblingService");
        return m_scrobblingServices.contains(typeid(T).hash_code());
    }

    void newTrackLoaded(TrackPointer pTrack) override;
    void trackUnloaded(TrackPointer pTrack) override;
    void slotNowListening(TrackPointer pTrack) override;
    void slotAttemptScrobble(TrackPointer pTrack) override;
    void slotAllTracksPaused() override;
    void guiTick(double timeSinceLastTick);

  private:
    unsigned int m_gracePeriodSeconds;
    QHash<TrackId, GracePeriod> m_trackedTracks;
    QHash<size_t, ScrobblingServicePtr> m_scrobblingServices;
};
