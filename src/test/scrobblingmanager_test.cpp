#include "test/scrobblingmanager_test.h"

#include <QObject>
#include <QString>
#include <functional>
#include <utility>

#include "broadcast/scrobblingmanager.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "mixer/basetrackplayer.h"
#include "mixer/deck.h"
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
    MOCK_CONST_METHOD1(isPlayerAudible, bool(BaseTrackPlayer*));
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
        // Set up left player
        QObject::connect(&dummyPlayerLeft,
                &Deck::newTrackLoaded,
                [this](TrackPointer pTrack) -> void {
                    scrobblingManager.slotNewTrackLoaded(
                            pTrack, "DummyPlayerLeft");
                });
        QObject::connect(&dummyPlayerLeft,
                &Deck::trackResumed,
                [this](TrackPointer pTrack) -> void {
                    scrobblingManager.slotTrackResumed(
                            pTrack, "DummyPlayerLeft");
                });
        QObject::connect(&dummyPlayerLeft,
                &Deck::trackPaused,
                &scrobblingManager,
                &ScrobblingManager::slotTrackPaused);
        // Set up right player
        QObject::connect(&dummyPlayerRight,
                &Deck::newTrackLoaded,
                [this](TrackPointer pTrack) -> void {
                    scrobblingManager.slotNewTrackLoaded(
                            pTrack, "DummyPlayerRight");
                });
        QObject::connect(&dummyPlayerRight,
                &Deck::trackResumed,
                [this](TrackPointer pTrack) -> void {
                    scrobblingManager.slotTrackResumed(
                            pTrack, "DummyPlayerRight");
                });
        QObject::connect(&dummyPlayerRight,
                &Deck::trackPaused,
                &scrobblingManager,
                &ScrobblingManager::slotTrackPaused);
        EXPECT_CALL(*playerManagerMock, getPlayer(QString("DummyPlayerLeft")))
                .WillRepeatedly(testing::Return(&dummyPlayerLeft));
        EXPECT_CALL(*playerManagerMock, getPlayer(QString("DummyPlayerRight")))
                .WillRepeatedly(testing::Return(&dummyPlayerRight));
    }

    ~ScrobblingTest() {
        delete playerManagerMock;
    }

    PlayerManagerMock* playerManagerMock;
    ScrobblingManager scrobblingManager;
    PlayerMock dummyPlayerLeft, dummyPlayerRight;
    TrackPointer dummyTrackLeft, dummyTrackRight;
    RegularTimerMock* timerScrobbler;
    MetadataBroadcasterMock* broadcastMock;
    AudibleStrategyMock* strategyMock;
};

// 1 track, audible the whole time
TEST_F(ScrobblingTest, SingleTrackAudible) {
    std::function<std::shared_ptr<TrackTimingInfo>(TrackPointer)> factory;
    factory = [this](TrackPointer pTrack) -> std::shared_ptr<TrackTimingInfo> {
        Q_UNUSED(pTrack);
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
    };
    scrobblingManager.setTrackInfoFactory(factory);
    EXPECT_CALL(*strategyMock, isPlayerAudible(_))
            .WillOnce(testing::Return(true));
    EXPECT_CALL(*broadcastMock, slotAttemptScrobble(_));
    dummyPlayerLeft.emitTrackLoaded(dummyTrackLeft);
    dummyPlayerLeft.emitTrackResumed(dummyTrackLeft);
}

// 1 Track, inaudible.
TEST_F(ScrobblingTest, SingleTrackInaudible) {
    std::function<std::shared_ptr<TrackTimingInfo>(TrackPointer)> factory;
    factory = [this](TrackPointer pTrack) -> std::shared_ptr<TrackTimingInfo> {
        Q_UNUSED(pTrack);
        std::shared_ptr<TrackTimingInfo>
                trackInfo(new TrackTimingInfo(pTrack));
        trackInfo->setTimer(new testing::NiceMock<RegularTimerMock>);
        trackInfo->setElapsedTimer(new testing::NiceMock<ElapsedTimerMock>);
        return trackInfo;
    };
    scrobblingManager.setTrackInfoFactory(factory);
    EXPECT_CALL(*strategyMock, isPlayerAudible(_))
            .WillOnce(testing::Return(false));
    dummyPlayerLeft.emitTrackLoaded(dummyTrackLeft);
    dummyPlayerLeft.emitTrackResumed(dummyTrackLeft);
    ASSERT_FALSE(scrobblingManager.hasScrobbledAnyTrack());
}

// Doesn't work because the two Id's are -1 and Scrobbling
//  Manager stores ID's not TrackPointers.
TEST_F(ScrobblingTest, DISABLED_TwoTracksUnbalanced) {
    std::function<std::shared_ptr<TrackTimingInfo>(TrackPointer)> factory;
    factory = [this](TrackPointer pTrack) -> std::shared_ptr<TrackTimingInfo> {
        if (pTrack == dummyTrackLeft) {
            std::shared_ptr<TrackTimingInfo>
                    trackInfo(new TrackTimingInfo(pTrack));
            trackInfo->setTimer(new testing::NiceMock<RegularTimerMock>);
            trackInfo->setElapsedTimer(new testing::NiceMock<ElapsedTimerMock>);
            return trackInfo;
        } else {
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
    };
    scrobblingManager.setTrackInfoFactory(factory);
    EXPECT_CALL(*strategyMock, isPlayerAudible(&dummyPlayerLeft))
            .WillOnce(testing::Return(false));
    EXPECT_CALL(*strategyMock, isPlayerAudible(&dummyPlayerRight))
            .WillOnce(testing::Return(true));
    EXPECT_CALL(*broadcastMock, slotAttemptScrobble(dummyTrackRight));
    dummyPlayerLeft.emitTrackLoaded(dummyTrackLeft);
    dummyPlayerRight.emitTrackLoaded(dummyTrackRight);
    dummyPlayerLeft.emitTrackResumed(dummyTrackLeft);
    dummyPlayerRight.emitTrackResumed(dummyTrackRight);
}
