#include "test/scrobblingmanager_test.h"

#include <QObject>
#include <QString>
#include <functional>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "mixer/basetrackplayer.h"
#include "mixer/playermanager.h"
#include "track/track.h"
#include "track/trackplaytimers.h"

using testing::_;

class PlayerManagerMock : public PlayerManagerInterface {
  public:
    ~PlayerManagerMock() = default;
    MOCK_CONST_METHOD1(getPlayer, BaseTrackPlayer*(QString));
    MOCK_CONST_METHOD1(getDeck, Deck*(unsigned int));
    MOCK_CONST_METHOD0(numberOfDecks, unsigned int());
    MOCK_CONST_METHOD1(getPreviewDeck, PreviewDeck*(unsigned int));
    MOCK_CONST_METHOD0(numberOfPreviewDecks, unsigned int());
    MOCK_CONST_METHOD1(getSampler, Sampler*(unsigned int));
    MOCK_CONST_METHOD0(numberOfSamplers, unsigned int());
};

class ElapsedTimerMock : public TrackTimers::ElapsedTimer {
  public:
    ~ElapsedTimerMock() override = default;
    MOCK_METHOD0(invalidate, void());
    MOCK_CONST_METHOD0(isValid, bool());
    MOCK_METHOD0(start, void());
    MOCK_CONST_METHOD0(elapsed, qint64());
};

class AudibleStrategyMock : public TrackAudibleStrategy {
  public:
    ~AudibleStrategyMock() override = default;
    MOCK_CONST_METHOD2(isTrackAudible, bool(TrackPointer, BaseTrackPlayer*));
};

PlayerMock::PlayerMock(QObject* pParent, const QString& group)
        : BaseTrackPlayer(pParent, group) {
}

class ScrobblingTest : public ::testing::Test {
  public:
    ScrobblingTest()
            : playerManagerMock(new PlayerManagerMock),
              scrobblingManager(playerManagerMock),
              dummyPlayerLeft(nullptr, "DummyPlayerLeft"),
              dummyPlayerRight(nullptr, "DummyPlayerRight"),
              dummyTrackLeft(Track::newDummy(QFileInfo(), TrackId())),
              dummyTrackRight(Track::newDummy(QFileInfo(), TrackId())),
              timerScrobbler(new RegularTimerMock),
              broadcastMock(new testing::NiceMock<MetadataBroadcasterMock>),
              strategyMock(new AudibleStrategyMock) {
        scrobblingManager.setAudibleStrategy(strategyMock);
        scrobblingManager.setMetadataBroadcaster(broadcastMock);
        scrobblingManager.setTimer(timerScrobbler);
        dummyTrackLeft->setDuration(120);
        dummyTrackRight->setDuration(120);
        //Set up left player
        QObject::connect(&dummyPlayerLeft, SIGNAL(newTrackLoaded(TrackPointer)), &scrobblingManager, SLOT(slotNewTrackLoaded(TrackPointer)));
        QObject::connect(&dummyPlayerLeft, SIGNAL(trackResumed(TrackPointer)), &scrobblingManager, SLOT(slotTrackResumed(TrackPointer)));
        QObject::connect(&dummyPlayerLeft, SIGNAL(trackPaused(TrackPointer)), &scrobblingManager, SLOT(slotTrackPaused(TrackPointer)));
        //Set up right player
        QObject::connect(&dummyPlayerRight, SIGNAL(newTrackLoaded(TrackPointer)), &scrobblingManager, SLOT(slotNewTrackLoaded(TrackPointer)));
        QObject::connect(&dummyPlayerRight, SIGNAL(trackResumed(TrackPointer)), &scrobblingManager, SLOT(slotTrackResumed(TrackPointer)));
        QObject::connect(&dummyPlayerRight, SIGNAL(trackPaused(TrackPointer)), &scrobblingManager, SLOT(slotTrackPaused(TrackPointer)));
    }

    ~ScrobblingTest() {
        delete playerManagerMock;
    }

    PlayerManagerMock* playerManagerMock;
    ScrobblingManager scrobblingManager;
    PlayerMock dummyPlayerLeft, dummyPlayerRight;
    TrackPointer dummyTrackLeft, dummyTrackRight;
    RegularTimerMock* timerScrobbler;
    testing::NiceMock<MetadataBroadcasterMock>* broadcastMock;
    AudibleStrategyMock* strategyMock;
};

//1 track, audible the whole time
TEST_F(ScrobblingTest, SingleTrackAudible) {
    std::function<std::shared_ptr<TrackTimingInfo>(TrackPointer)> factory;
    factory = [this](TrackPointer pTrack) -> std::shared_ptr<TrackTimingInfo> {
        if (pTrack == dummyTrackLeft) {
            std::shared_ptr<TrackTimingInfo>
                    trackInfo(new TrackTimingInfo(pTrack));
            ElapsedTimerMock* etMock = new ElapsedTimerMock;
            EXPECT_CALL(*etMock, invalidate());
            EXPECT_CALL(*etMock, isValid())
                    .WillOnce(testing::Return(false))
                    .WillOnce(testing::Return(true));
            EXPECT_CALL(*etMock, start());
            EXPECT_CALL(*etMock, elapsed())
                    .WillOnce(testing::Return(60000));
            RegularTimerMock* tMock = new RegularTimerMock;
            EXPECT_CALL(*tMock, start(1000))
                    .WillOnce(testing::InvokeWithoutArgs(
                            trackInfo.get(),
                            &TrackTimingInfo::slotCheckIfScrobbable));
            trackInfo->setTimer(tMock);
            trackInfo->setElapsedTimer(etMock);
            return trackInfo;
        }
        throw;
    };
    scrobblingManager.setTrackInfoFactory(factory);
    EXPECT_CALL(*strategyMock, isTrackAudible(_, _))
            .WillOnce(testing::Return(true));
    EXPECT_CALL(*broadcastMock, slotAttemptScrobble(_));
    dummyPlayerLeft.emitTrackLoaded(dummyTrackLeft);
    dummyPlayerLeft.emitTrackResumed(dummyTrackLeft);
}