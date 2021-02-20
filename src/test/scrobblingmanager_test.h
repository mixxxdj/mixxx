#pragma once

#include <QObject>

#include "broadcast/metadatabroadcast.h"
#include "gmock/gmock.h"
#include "mixer/basetrackplayer.h"

class MetadataBroadcasterMock : public MetadataBroadcasterInterface {
    Q_OBJECT
  public:
    ~MetadataBroadcasterMock() = default;
    MOCK_METHOD1(slotNowListening, void(TrackPointer));
    MOCK_METHOD1(slotAttemptScrobble, void(TrackPointer));
    MOCK_METHOD0(slotAllTracksPaused, void());
    MetadataBroadcasterInterface&
    addNewScrobblingService(const ScrobblingServicePtr& ptr) override {
        return *this;
    }
    MOCK_METHOD1(newTrackLoaded, void(TrackPointer));
    MOCK_METHOD1(trackUnloaded, void(TrackPointer));
    MOCK_METHOD1(guiTick, void(double));
};

class RegularTimerMock : public TrackTimers::RegularTimer {
    Q_OBJECT
  public:
    ~RegularTimerMock() = default;
    MOCK_METHOD1(start, void(double));
    MOCK_CONST_METHOD0(isActive, bool());
    MOCK_METHOD0(stop, void());
};

class PlayerMock : public BaseTrackPlayer {
    Q_OBJECT
  public:
    PlayerMock(QObject* pParent, const QString& group);
    ~PlayerMock() = default;
    MOCK_CONST_METHOD0(getLoadedTrack, TrackPointer());
    MOCK_METHOD2(slotLoadTrack, void(TrackPointer, bool));
    void emitTrackLoaded(TrackPointer pTrack) {
        emit(newTrackLoaded(pTrack));
    }
    void emitTrackResumed(TrackPointer pTrack) {
        emit(trackResumed(pTrack));
    }
    void emitTrackPaused(TrackPointer pTrack) {
        emit(trackPaused(pTrack));
    }
};
