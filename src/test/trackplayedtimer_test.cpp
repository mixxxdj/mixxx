#include "test/trackplayedtimer_test.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "track/track.h"
#include "track/trackplaytimers.h"
#include "track/tracktiminginfo.h"

class ElapsedTimerMock : public TrackTimers::ElapsedTimer {
  public:
    ~ElapsedTimerMock() = default;
    MOCK_METHOD0(invalidate, void());
    MOCK_METHOD0(start, void());
    MOCK_CONST_METHOD0(isValid, bool());
    MOCK_CONST_METHOD0(elapsed, qint64());
};

class DISABLED_TrackTimingInfoTest : public testing::Test {
  public:
    DISABLED_TrackTimingInfoTest()
            : trackInfo(TrackPointer()) {
        testTrack = Track::newDummy(QFileInfo(), TrackId());
        trackInfo.setTrackPointer(testTrack);
    }
    ~DISABLED_TrackTimingInfoTest() = default;
    TrackPointer testTrack;
    TrackTimingInfo trackInfo;
};

TEST_F(DISABLED_TrackTimingInfoTest, SendsSignalWhenScrobbable) {
    testTrack->setDuration(5);
    // These have to be created in the heap otherwise
    // we're deleting them twice.
    ElapsedTimerMock* etmock = new ElapsedTimerMock();
    TimerMock* tmock = new TimerMock();
    EXPECT_CALL(*etmock, invalidate())
            .Times(2);
    EXPECT_CALL(*etmock, isValid())
            .WillOnce(testing::Return(false))
            .WillOnce(testing::Return(true));
    EXPECT_CALL(*etmock, start());
    EXPECT_CALL(*tmock, start(1000))
            .WillOnce(testing::InvokeWithoutArgs(&trackInfo,
                    &TrackTimingInfo::slotCheckIfScrobbable));
    EXPECT_CALL(*etmock, elapsed())
            .WillOnce(testing::Return(2500));
    trackInfo.setTimer(tmock);
    trackInfo.setElapsedTimer(etmock);
    trackInfo.resetPlayedTime();
    trackInfo.resumePlayedTime();
    ASSERT_TRUE(trackInfo.isScrobbable());
}
