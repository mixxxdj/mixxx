// Tests for enginebuffer.cpp

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QtDebug>
#include <QTest>

#include "basetrackplayer.h"
#include "configobject.h"
#include "controlobject.h"
#include "test/mockedenginebackendtest.h"
#include "test/mixxxtest.h"


class EngineBufferTest : public MockedEngineBackendTest {
};

TEST_F(EngineBufferTest, DisableKeylockResetsPitch) {
    // To prevent one-slider users from getting stuck on a key, unsetting
    // keylock resets the musical pitch.
    ControlObject::set(ConfigKey(m_sGroup1, "keylockMode"), 1.0);  // kLockCurrentKey;
    ControlObject::set(ConfigKey(m_sGroup1, "file_bpm"), 128.0);
    ControlObject::set(ConfigKey(m_sGroup1, "keylock"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup1, "pitch"),0.5);
    ProcessBuffer();

    ControlObject::set(ConfigKey(m_sGroup1, "keylock"), 0.0);
    // We require a buffer process to see that the keylock state has changed.
    ProcessBuffer();
    ASSERT_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "pitch")));
}

// TODO(xxx) make EngineBufferTest::TrackLoadResetsPitch() work
/* Does not work yet because we have no BaseTrackPlayerImpl in this test
TEST_F(EngineBufferTest, TrackLoadResetsPitch) {
    // When a new track is loaded, the pitch value should be reset.
    config()->set(ConfigKey("[Controls]","SpeedAutoReset"),
            ConfigValue(BaseTrackPlayer::RESET_PITCH));
    ControlObject::set(ConfigKey(m_sGroup1, "file_bpm"), 128.0);
    ControlObject::set(ConfigKey(m_sGroup1, "pitch"), 0.5);
    ProcessBuffer();
    ASSERT_EQ(0.5, ControlObject::get(ConfigKey(m_sGroup1, "pitch")));

    m_pChannel1->getEngineBuffer()->loadFakeTrack();
    ASSERT_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "pitch")));
}
*/

TEST_F(EngineBufferTest, PitchRoundtrip) {
    ControlObject::set(ConfigKey(m_sGroup1, "keylock"), 0.0);
    ControlObject::set(ConfigKey(m_sGroup1, "keylockMode"), 0.0); // kLockOriginalKey;
    ProcessBuffer();
    // we are in kPakmOffsetScaleReseting mode
    ControlObject::set(ConfigKey(m_sGroup1, "rate"),0.5);
    ProcessBuffer();
    // pitch must not change
    ASSERT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "pitch_adjust")));

    ControlObject::set(ConfigKey(m_sGroup1, "pitch_adjust"),0.5);
    ProcessBuffer();
    // rate must not change
    ASSERT_DOUBLE_EQ(0.5, ControlObject::get(ConfigKey(m_sGroup1, "rate")));

    ControlObject::set(ConfigKey(m_sGroup1, "keylock"), 1.0);
    ProcessBuffer();
    // pitch and speed must not change
    ASSERT_DOUBLE_EQ(0.5, ControlObject::get(ConfigKey(m_sGroup1, "pitch_adjust")));
    ASSERT_DOUBLE_EQ(0.5, ControlObject::get(ConfigKey(m_sGroup1, "rate")));

    ControlObject::set(ConfigKey(m_sGroup1, "keylockMode"), 1.0); // kLockCurrentKey;
    ProcessBuffer();
    // rate must not change
    ASSERT_DOUBLE_EQ(0.5, ControlObject::get(ConfigKey(m_sGroup1, "rate")));
    // pitch must reflect the absolute pitch
    ASSERT_NEAR(0.5, ControlObject::get(ConfigKey(m_sGroup1, "pitch")), 1e-10);

    ControlObject::set(ConfigKey(m_sGroup1, "keylockMode"), 0.0); // kLockOriginalKey;
    ProcessBuffer();
    // rate must not change
    ASSERT_NEAR(0.5, ControlObject::get(ConfigKey(m_sGroup1, "pitch")), 1e-10);
    // pitch must reflect the pitch shift only
    ASSERT_DOUBLE_EQ(0.5, ControlObject::get(ConfigKey(m_sGroup1, "rate")));
}

TEST_F(EngineBufferTest, SlowRubberBand) {
    // At very slow speeds, RubberBand needs to reallocate buffers and since this
    // is done in the engine thread it can be a major party-stopper.
    // Make sure slow speeds still use the linear scaler.
    ControlObject::set(ConfigKey(m_sGroup1, "pitch"), 2.8);

    // Hack to get a slow, non-scratching direct speed
    ControlObject::set(ConfigKey(m_sGroup1, "rateSearch"), 0.0072);

    // With Soundtouch, it should switch the scaler as well
    ControlObject::set(ConfigKey("[Master]", "keylock_engine"),
                       static_cast<double>(EngineBuffer::SOUNDTOUCH));
    ProcessBuffer();
    EXPECT_EQ(m_pMockScaleVinyl1, m_pChannel1->getEngineBuffer()->m_pScale);

    // Back to full speed
    ControlObject::set(ConfigKey(m_sGroup1, "rateSearch"), 1);
    ProcessBuffer();

    // With Rubberband, and transport stopped it should be still keylock
    ControlObject::set(ConfigKey("[Master]", "keylock_engine"),
                       static_cast<double>(EngineBuffer::RUBBERBAND));
    ControlObject::set(ConfigKey(m_sGroup1, "rateSearch"), 0.0);
    ProcessBuffer();
    EXPECT_EQ(m_pMockScaleKeylock1, m_pChannel1->getEngineBuffer()->m_pScale);

    ControlObject::set(ConfigKey(m_sGroup1, "rateSearch"), 0.0072);

    // Paying at low rate, the vinyl scaler should be used
    ProcessBuffer();
    EXPECT_EQ(m_pMockScaleVinyl1, m_pChannel1->getEngineBuffer()->m_pScale);
}


TEST_F(EngineBufferTest, ScalerNoTransport) {
    // normaly use the Vinyl scaler
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ProcessBuffer();
    EXPECT_EQ(m_pMockScaleVinyl1, m_pChannel1->getEngineBuffer()->m_pScale);

    // switch to keylock scaler
    ControlObject::set(ConfigKey(m_sGroup1, "keylock"), 1.0);
    ProcessBuffer();
    EXPECT_EQ(m_pMockScaleKeylock1, m_pChannel1->getEngineBuffer()->m_pScale);

    // Stop and disable keylock: do not change scaler
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 0.0);
    ControlObject::set(ConfigKey(m_sGroup1, "keylock"), 0.0);
    ProcessBuffer();
    EXPECT_EQ(m_pMockScaleKeylock1, m_pChannel1->getEngineBuffer()->m_pScale);

    // play: we need to use vinyl scaler
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ProcessBuffer();
    EXPECT_EQ(m_pMockScaleVinyl1, m_pChannel1->getEngineBuffer()->m_pScale);
}

TEST_F(EngineBufferTest, VinylScalerRampZero) {
    // scratch in play mode
    ControlObject::set(ConfigKey(m_sGroup1, "scratch2_enable"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup1, "scratch2"), 1.0);

    ProcessBuffer();
    EXPECT_EQ(m_pMockScaleVinyl1, m_pChannel1->getEngineBuffer()->m_pScale);
    EXPECT_EQ(m_pMockScaleVinyl1->getProcessedTempo(), 1.0);

    ControlObject::set(ConfigKey(m_sGroup1, "scratch2"), 0.0);

    // we are in scratching mode so a zero rate has to be processed
    ProcessBuffer();
    EXPECT_EQ(m_pMockScaleVinyl1, m_pChannel1->getEngineBuffer()->m_pScale);
    EXPECT_EQ(m_pMockScaleVinyl1->getProcessedTempo(), 0.0);
}

TEST_F(EngineBufferTest, ReadFadeOut) {
    // Start playing
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);

    ProcessBuffer();
    EXPECT_EQ(m_pMockScaleVinyl1, m_pChannel1->getEngineBuffer()->m_pScale);
    EXPECT_EQ(m_pMockScaleVinyl1->getProcessedTempo(), 1.0);

    // pause
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 0.0);

    // The scaler need to be processed with the old rate to
    // prepare the crossfade buffer
    ProcessBuffer();
    EXPECT_EQ(m_pMockScaleVinyl1, m_pChannel1->getEngineBuffer()->m_pScale);
    EXPECT_EQ(m_pMockScaleVinyl1->getProcessedTempo(), 1.0);
}

TEST_F(EngineBufferTest, ResetPitchAdjustUsesLinear) {
    // If the key was adjusted, but keylock is off, and then the key is
    // reset, then the engine should be using the linear scaler.

    // First, we should be using the keylock scaler here.
    ControlObject::set(ConfigKey(m_sGroup1, "pitch"), 1.2);
    // Remember that a rate of "0" is "regular playback speed".
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), 0.05);
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ProcessBuffer();
    EXPECT_EQ(m_pMockScaleKeylock1, m_pChannel1->getEngineBuffer()->m_pScale);

    // Reset pitch adjust, and we should be back to the linear scaler.
    ControlObject::set(ConfigKey(m_sGroup1, "pitch_adjust_set_default"), 1.0);
    ProcessBuffer();
    EXPECT_EQ(m_pMockScaleVinyl1, m_pChannel1->getEngineBuffer()->m_pScale);
}

TEST_F(EngineBufferTest, SoundTouchCrashTest) {
    // Soundtouch has a bug where a pitch value of zero causes an infinite loop
    // and crash.

    // We actually have to load a track to test this.
    const QString kGroup4 = "[Channel4]";
    EngineDeck* channel4 = new EngineDeck(
                m_pEngineMaster->registerChannelGroup(kGroup4),
                m_pConfig.data(), m_pEngineMaster, m_pEffectsManager,
                EngineChannel::CENTER);
    addDeck(channel4);
    // This file comes from the autodjprocessor test.
    const QString kTrackLocationTest(QDir::currentPath() +
                                 "/src/test/id3-test-data/cover-test.mp3");
    TrackPointer pTrack(new TrackInfoObject(kTrackLocationTest));
    channel4->getEngineBuffer()->slotLoadTrack(pTrack, true);

    // Wait for the track to load.
    ProcessBuffer();
    for (int i = 0; i < 10 && !channel4->getEngineBuffer()->isTrackLoaded();
            ++i) {
        QTest::qSleep(1000); // millis
    }
    ASSERT_TRUE(channel4->getEngineBuffer()->isTrackLoaded());

    ControlObject::set(ConfigKey("[Master]", "keylock_engine"),
                       static_cast<double>(EngineBuffer::SOUNDTOUCH));
    ControlObject::set(ConfigKey(kGroup4, "pitch"), 1.2);
    ControlObject::set(ConfigKey(kGroup4, "rate"), 0.05);
    ControlObject::set(ConfigKey(kGroup4, "play"), 1.0);
    // Start by playing with soundtouch enabled.
    ProcessBuffer();
    // Pause the buffer.  This causes the pitch to be set to 0.
    ControlObject::set(ConfigKey(kGroup4, "play"), 0.0);
    ProcessBuffer();
    ControlObject::set(ConfigKey(kGroup4, "rateSearch"), -0.05);
    // Should not crash
    ProcessBuffer();
}
