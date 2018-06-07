#include "broadcast/scrobblingmanager.h"

#include <QString>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "mixer/basetrackplayer.h"
#include "mixer/playermanager.h"
#include "test/scrobblingmanager_test.h"
#include "track/track.h"
#include "track/trackplaytimers.h"

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
    ~ElapsedTimerMock() = default;
    MOCK_METHOD0(invalidate, void());
    MOCK_CONST_METHOD0(isValid, bool());
    MOCK_METHOD0(start, void());
    MOCK_CONST_METHOD0(elapsed, qint64());
};

class AudibleStrategyMock : public TrackAudibleStrategy {
  public:
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
              elapsedTimerLeft(new ElapsedTimerMock),
              elapsedTimerRight(new ElapsedTimerMock),
              timerLeft(new RegularTimerMock),
              timerRight(new RegularTimerMock),
              timerScrobbler(new RegularTimerMock),
              broadcastMock(new MetadataBroadcasterMock),
              strategyMock(new AudibleStrategyMock) {
        scrobblingManager.setMetadataBroadcaster(broadcastMock);
        scrobblingManager.setTimer(timerScrobbler);
        scrobblingManager.setAudibleStrategy(strategyMock);
    }

    PlayerManagerMock* playerManagerMock;
    ScrobblingManager scrobblingManager;
    PlayerMock dummyPlayerLeft, dummyPlayerRight;
    TrackPointer dummyTrackLeft, dummyTrackRight;
    ElapsedTimerMock *elapsedTimerLeft, *elapsedTimerRight;
    RegularTimerMock *timerLeft, *timerRight, *timerScrobbler;
    MetadataBroadcasterMock* broadcastMock;
    AudibleStrategyMock* strategyMock;
};

//1 track, audible the whole time
TEST_F(ScrobblingTest, SingleTrackAudible) {
}