#include <gtest/gtest.h>
#include <QtDebug>

#include "track/playcounter.h"
#include "util/math.h"

class PlayCounterTest : public testing::Test {
  protected:
    void updatePlayedAndVerify(PlayCounter* pPlayCounter, bool bPlayed) {
        bool isPlayedBefore = pPlayCounter->isPlayed();
        int timesPlayedBefore = pPlayCounter->getTimesPlayed();
        pPlayCounter->updateLastPlayedNowAndTimesPlayed(bPlayed);
        bool isPlayedAfter = pPlayCounter->isPlayed();
        int timesPlayedAfter = pPlayCounter->getTimesPlayed();
        if (bPlayed) {
            EXPECT_TRUE(isPlayedAfter);
            EXPECT_EQ(timesPlayedBefore + 1, timesPlayedAfter);
        } else {
            EXPECT_FALSE(isPlayedAfter);
            if (isPlayedBefore) {
                EXPECT_EQ(math_max(0, timesPlayedBefore - 1), timesPlayedAfter);
            } else {
                EXPECT_EQ(timesPlayedBefore, timesPlayedAfter);
            }
        }
    }

    void resetAndVerify(PlayCounter* pPlayCounter) {
        *pPlayCounter = PlayCounter();
        bool isPlayedAfter = pPlayCounter->isPlayed();
        int timesPlayedAfter = pPlayCounter->getTimesPlayed();
        EXPECT_FALSE(isPlayedAfter);
        EXPECT_EQ(0, timesPlayedAfter);
    }

    void testCycle(PlayCounter* pPlayCounter) {
        updatePlayedAndVerify(pPlayCounter, false);
        updatePlayedAndVerify(pPlayCounter, true);
        updatePlayedAndVerify(pPlayCounter, false);
        updatePlayedAndVerify(pPlayCounter, true);
        updatePlayedAndVerify(pPlayCounter, true);
        resetAndVerify(pPlayCounter);
        updatePlayedAndVerify(pPlayCounter, true);
        updatePlayedAndVerify(pPlayCounter, false);
        updatePlayedAndVerify(pPlayCounter, false);
        updatePlayedAndVerify(pPlayCounter, true);
        updatePlayedAndVerify(pPlayCounter, true);
        updatePlayedAndVerify(pPlayCounter, false);
        EXPECT_EQ(1, pPlayCounter->getTimesPlayed());
        EXPECT_FALSE(pPlayCounter->isPlayed());
        resetAndVerify(pPlayCounter);
        ASSERT_EQ(PlayCounter(), *pPlayCounter);
    }
};

TEST_F(PlayCounterTest, DefaultCounstructor) {
    PlayCounter playCounter;

    EXPECT_FALSE(playCounter.isPlayed());
    EXPECT_EQ(0, playCounter.getTimesPlayed());

    testCycle(&playCounter);
}

TEST_F(PlayCounterTest, InitializingCounstructor) {
    PlayCounter playCounter(5);

    EXPECT_FALSE(playCounter.isPlayed());
    EXPECT_EQ(5, playCounter.getTimesPlayed());

    testCycle(&playCounter);
}
