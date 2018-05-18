#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "track/track.h"
#include "track/trackplaytimers.h"

class TimerMock : public Timer {
  public:
    MOCK_METHOD1(start,void(int msec));
    MOCK_METHOD0(isActive,bool());
    MOCK_METHOD0(stop,void());
};

class ElapsedTimerMock : public ElapsedTimer {
  public:
    MOCK_METHOD0(invalidate,void());
    MOCK_METHOD0(start,void());
    MOCK_METHOD0(isValid,bool());
    MOCK_METHOD0(elapsed,qint64());
};


class TrackTest : public testing::Test, public QObject {
    Q_OBJECT
  public:
    TrackTest() : scrobbled(false) {
        testTrack = Track::newDummy(QFileInfo(),TrackId());        
    }
    TrackPointer testTrack;
  public slots:
    void slotTrackScrobbable(Track *pTrack) {
        scrobbled = true;
    }
    bool scrobbled;
};

TEST_F(TrackTest,SendsSignalWhenScrobbable) {
    testTrack->setDuration(5);
    ElapsedTimerMock etmock;
    TimerMock tmock;
    EXPECT_CALL(etmock,invalidate());
    EXPECT_CALL(etmock,isValid())
        .WillOnce(Return(false));
    testTrack->setTimer(&tmock);
    testTrack->setElapsedTimer(&etmock);
}