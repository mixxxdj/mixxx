#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "track/track.h"
#include "track/trackplaytimers.h"
#include "test/trackplayedtimer_test.h"

class ElapsedTimerMock : public TrackTimers::ElapsedTimer {
  public:
    ~ElapsedTimerMock() = default;
    MOCK_METHOD0(invalidate,void());
    MOCK_METHOD0(start,void());
    MOCK_METHOD0(isValid,bool());
    MOCK_METHOD0(elapsed,qint64());
};


class TrackTest : public testing::Test {
  public:
    TrackTest() {
        testTrack = Track::newDummy(QFileInfo(),TrackId());        
    }
    ~TrackTest() = default;
    TrackPointer testTrack;
  public slots:
};

TEST_F(TrackTest,SendsSignalWhenScrobbable) {
    testTrack->setDuration(5);
    ElapsedTimerMock etmock;
    TimerMock tmock;
    EXPECT_CALL(etmock,invalidate());
    EXPECT_CALL(etmock,isValid())
        .WillOnce(testing::Return(false))
        .WillOnce(testing::Return(true));
    EXPECT_CALL(etmock,start());
    EXPECT_CALL(tmock,start(1000))
        .WillOnce(testing::InvokeWithoutArgs(testTrack.get(),
                  &Track::slotCheckIfScrobbable));
    EXPECT_CALL(etmock,elapsed())
        .WillOnce(testing::Return(2500));   
    testTrack->setTimer(&tmock);
    testTrack->setElapsedTimer(&etmock);
    testTrack->resetPlayedTime();
    testTrack->resumePlayedTime();
    ASSERT_TRUE(testTrack->isScrobbable());
}