// Tests for enginebuffer.cpp

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QtDebug>
#include <QTest>

#include "mixer/basetrackplayer.h"
#include "preferences/usersettings.h"
#include "control/controlobject.h"
#include "test/mockedenginebackendtest.h"
#include "test/mixxxtest.h"
#include "test/signalpathtest.h"
#include "engine/controls/ratecontrol.h"

// In case any of the test in this file fail. You can use the audioplot.py tool
// in the scripts folder to visually compare the results of the enginebuffer
// with the golden test data.

class EngineBufferTest : public MockedEngineBackendTest {};

class EngineBufferE2ETest : public SignalPathTest {};

TEST_F(EngineBufferTest, DisableKeylockResetsPitch) {
    // To prevent one-slider users from getting stuck on a key, unsetting
    // keylock resets the musical pitch.
    ControlObject::set(ConfigKey(m_sGroup1, "keylockMode"),
                       1.0);  // kLockCurrentKey;
    ControlObject::set(ConfigKey(m_sGroup1, "file_bpm"), 128.0);
    ControlObject::set(ConfigKey(m_sGroup1, "keylock"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup1, "pitch"), 0.5);
    ProcessBuffer();

    ControlObject::set(ConfigKey(m_sGroup1, "keylock"), 0.0);
    // We require a buffer process to see that the keylock state has changed.
    ProcessBuffer();
    ASSERT_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "pitch")));
}

TEST_F(EngineBufferTest, TrackLoadResetsPitch) {
    // When a new track is loaded, the pitch value should be reset.
    config()->set(ConfigKey("[Controls]","SpeedAutoReset"),
                  ConfigValue(BaseTrackPlayer::RESET_PITCH));
    ControlObject::set(ConfigKey(m_sGroup1, "file_bpm"), 128.0);
    ControlObject::set(ConfigKey(m_sGroup1, "pitch_adjust"), 0.5);
    ProcessBuffer();
    ASSERT_NEAR(0.5, ControlObject::get(ConfigKey(m_sGroup1, "pitch_adjust")), 1e-10);

    m_pMixerDeck1->loadFakeTrack(false, 0.0);
    ASSERT_NEAR(0.0, ControlObject::get(ConfigKey(m_sGroup1, "pitch_adjust")), 1e-10);
}

TEST_F(EngineBufferTest, PitchRoundtrip) {
    ControlObject::set(ConfigKey(m_sGroup1, "keylock"), 0.0);
    ControlObject::set(ConfigKey(m_sGroup1, "keylockMode"),
                       0.0);  // kLockOriginalKey;
    ProcessBuffer();
    // we are in kPakmOffsetScaleReseting mode
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), 0.5);
    ProcessBuffer();
    // pitch must not change
    ASSERT_DOUBLE_EQ(0.0,
                     ControlObject::get(ConfigKey(m_sGroup1, "pitch_adjust")));

    ControlObject::set(ConfigKey(m_sGroup1, "pitch_adjust"), 0.5);
    ProcessBuffer();
    // rate must not change
    ASSERT_DOUBLE_EQ(0.5, ControlObject::get(ConfigKey(m_sGroup1, "rate")));

    ControlObject::set(ConfigKey(m_sGroup1, "keylock"), 1.0);
    ProcessBuffer();
    // pitch and speed must not change
    ASSERT_DOUBLE_EQ(0.5,
                     ControlObject::get(ConfigKey(m_sGroup1, "pitch_adjust")));
    ASSERT_DOUBLE_EQ(0.5, ControlObject::get(ConfigKey(m_sGroup1, "rate")));

    ControlObject::set(ConfigKey(m_sGroup1, "keylockMode"),
                       1.0);  // kLockCurrentKey;
    ProcessBuffer();
    // rate must not change
    ASSERT_DOUBLE_EQ(0.5, ControlObject::get(ConfigKey(m_sGroup1, "rate")));
    // pitch must reflect the absolute pitch
    ASSERT_NEAR(0.5, ControlObject::get(ConfigKey(m_sGroup1, "pitch")), 1e-10);

    ControlObject::set(ConfigKey(m_sGroup1, "keylockMode"),
                       0.0);  // kLockOriginalKey;
    ProcessBuffer();
    // rate must not change
    ASSERT_NEAR(0.5, ControlObject::get(ConfigKey(m_sGroup1, "pitch")), 1e-10);
    // pitch must reflect the pitch shift only
    ASSERT_DOUBLE_EQ(0.5, ControlObject::get(ConfigKey(m_sGroup1, "rate")));
}

TEST_F(EngineBufferTest, SlowRubberBand) {
    // At very slow speeds, RubberBand needs to reallocate buffers and since
    // this
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
    // normally use the Vinyl scaler
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

TEST_F(EngineBufferE2ETest, SoundTouchCrashTest) {
    // Soundtouch has a bug where a pitch value of zero causes an infinite loop
    // and crash.
    ControlObject::set(ConfigKey("[Master]", "keylock_engine"),
                       static_cast<double>(EngineBuffer::SOUNDTOUCH));
    ControlObject::set(ConfigKey(m_sGroup1, "pitch"), 1.2);
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), 0.05);
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    // Start by playing with soundtouch enabled.
    ProcessBuffer();
    // Pause the buffer.  This causes the pitch to be set to 0.
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 0.0);
    ProcessBuffer();
    ControlObject::set(ConfigKey(m_sGroup1, "rateSearch"), -0.05);
    // Should not crash
    ProcessBuffer();
}

TEST_F(EngineBufferE2ETest, BasicProcessingTest) {
    // Confirm that playing ramps from silence, pausing ramps to silence,
    // and also just confirm that playing works as predicted.
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), 0.05);
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ProcessBuffer();
    assertBufferMatchesReference(m_pEngineMaster->masterBuffer(),
                                 kProcessBufferSize, "BasicProcessingTestPlay");
    ProcessBuffer();
    assertBufferMatchesReference(m_pEngineMaster->masterBuffer(),
                                 kProcessBufferSize, "BasicProcessingTestPlaying");
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 0.0);
    ProcessBuffer();
    assertBufferMatchesReference(m_pEngineMaster->masterBuffer(),
                                 kProcessBufferSize, "BasicProcessingTestPause");
}

TEST_F(EngineBufferE2ETest, ScratchTest) {
    // Confirm that vinyl scratching smoothly transitions from one direction
    // to the other.
    ControlObject::set(ConfigKey(m_sGroup1, "scratch2_enable"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup1, "scratch2"), 1.1);
    m_pChannel1->getEngineBuffer()->queueNewPlaypos(450,
                                                    EngineBuffer::SEEK_EXACT);
    ProcessBuffer();
    ControlObject::set(ConfigKey(m_sGroup1, "scratch2"), -1.1);
    ProcessBuffer();
    assertBufferMatchesReference(m_pEngineMaster->masterBuffer(),
                                 kProcessBufferSize, "ScratchTestMaster");
}

TEST_F(EngineBufferE2ETest, ScratchTestStart) {
    // Confirm that vinyl scratching smoothly transitions from zero speed
    // to an the other speed above the 0.07 threshold.
    ControlObject::set(ConfigKey(m_sGroup1, "scratch2_enable"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup1, "scratch2"), 1);
    ProcessBuffer();
    ControlObject::set(ConfigKey(m_sGroup1, "scratch2"), 0);
    ProcessBuffer();
    ControlObject::set(ConfigKey(m_sGroup1, "scratch2"), 0.5);
    ProcessBuffer();
    assertBufferMatchesReference(m_pEngineMaster->masterBuffer(),
                                 kProcessBufferSize, "ScratchTestStart");
}

TEST_F(EngineBufferE2ETest, ReverseTest) {
    // Confirm that pushing the reverse button smoothly transitions.
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), 0.0);
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ProcessBuffer();
    ControlObject::set(ConfigKey(m_sGroup1, "reverse"), 1.0);
    ProcessBuffer();
    assertBufferMatchesReference(m_pEngineMaster->masterBuffer(),
                                 kProcessBufferSize, "ReverseTest");
}

// DISABLED: This test is too dependent on the sound touch library version.
TEST_F(EngineBufferE2ETest, DISABLED_SoundTouchToggleTest) {
   // Test various cases where SoundTouch toggles on and off.
   ControlObject::set(ConfigKey("[Master]", "keylock_engine"),
                      static_cast<double>(EngineBuffer::SOUNDTOUCH));
   ControlObject::set(ConfigKey(m_sGroup1, "rate"), 0.5);
   ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
   ProcessBuffer();
   // Test transition from vinyl to keylock
   ControlObject::set(ConfigKey(m_sGroup1, "keylock"), 1.0);
   ProcessBuffer();
   assertBufferMatchesReference(m_pEngineMaster->masterBuffer(),
                                kProcessBufferSize, "SoundTouchTest");
   // Test transition from keylock to vinyl due to slow speed.
   ControlObject::set(ConfigKey(m_sGroup1, "play"), 0.0);
   ControlObject::set(ConfigKey(m_sGroup1, "rateSearch"), 0.0072);
   ProcessBuffer();
   assertBufferMatchesReference(m_pEngineMaster->masterBuffer(),
                                kProcessBufferSize, "SoundTouchTestSlow");
   // Test transition back to keylock due to regular speed.
   ControlObject::set(ConfigKey(m_sGroup1, "rateSearch"), 1.0);
   ProcessBuffer();
   assertBufferMatchesReference(m_pEngineMaster->masterBuffer(),
                                kProcessBufferSize, "SoundTouchTestRegular");
}

// DISABLED: This test is too dependent on the rubber band library version.
TEST_F(EngineBufferE2ETest, DISABLED_RubberbandToggleTest) {
   // Test various cases where Rubberband toggles on and off.
   ControlObject::set(ConfigKey("[Master]", "keylock_engine"),
                      static_cast<double>(EngineBuffer::RUBBERBAND));
   ControlObject::set(ConfigKey(m_sGroup1, "rate"), 0.5);
   ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
   ProcessBuffer();
   // Test transition from vinyl to keylock
   ControlObject::set(ConfigKey(m_sGroup1, "keylock"), 1.0);
   ProcessBuffer();
   assertBufferMatchesReference(m_pEngineMaster->masterBuffer(),
                                kProcessBufferSize, "RubberbandTest");
   // Test transition from keylock to vinyl due to slow speed.
   ControlObject::set(ConfigKey(m_sGroup1, "play"), 0.0);
   ControlObject::set(ConfigKey(m_sGroup1, "rateSearch"), 0.0072);
   ProcessBuffer();
   assertBufferMatchesReference(m_pEngineMaster->masterBuffer(),
                                kProcessBufferSize, "RubberbandTestSlow");
   // Test transition back to keylock due to regular speed.
   ControlObject::set(ConfigKey(m_sGroup1, "rateSearch"), 1.0);
   ProcessBuffer();
   assertBufferMatchesReference(m_pEngineMaster->masterBuffer(),
                                kProcessBufferSize, "RubberbandTestRegular");
}

// DISABLED: This test is too dependent on the sound touch library version.
// NOTE(uklotzde, 2018-01-10): We have also seen spurious failures on the
// Linux build server under high load. These failures might by caused by
// delayed asynchronous reads from CachingReader. The corresponding chunks
// will be filled with 0-samples by the engine buffer scaler.
TEST_F(EngineBufferE2ETest, DISABLED_KeylockReverseTest) {
    // Confirm that when toggling reverse while keylock is on, interpolation
    // is smooth.
    ControlObject::set(ConfigKey("[Master]", "keylock_engine"),
                       static_cast<double>(EngineBuffer::SOUNDTOUCH));
    ControlObject::set(ConfigKey(m_sGroup1, "keylockMode"),
                       0.0);
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), 0.5);
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup1, "keylock"), 1.0);
    ProcessBuffer();
    ControlObject::set(ConfigKey(m_sGroup1, "reverse"), 1.0);
    ProcessBuffer();
    assertBufferMatchesReference(m_pEngineMaster->masterBuffer(),
                                 kProcessBufferSize, "KeylockReverseTest");
}

TEST_F(EngineBufferE2ETest, SeekTest) {
    // Confirm that seeking to a new position smoothly transitions.
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), 0.0);
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ProcessBuffer();
    m_pChannel1->getEngineBuffer()->queueNewPlaypos(1000,
                                                    EngineBuffer::SEEK_EXACT);
    ProcessBuffer();
    assertBufferMatchesReference(m_pEngineMaster->masterBuffer(),
                                 kProcessBufferSize, "SeekTest");
}

TEST_F(EngineBufferE2ETest, SoundTouchReverseTest) {
    // This test must not crash when changing to reverse while pitch is tweaked
    // Testing bug #1458263
    ControlObject::set(ConfigKey("[Master]", "keylock_engine"),
                       static_cast<double>(EngineBuffer::SOUNDTOUCH));
    ControlObject::set(ConfigKey(m_sGroup1, "pitch"), -1);
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ProcessBuffer();
    ControlObject::set(ConfigKey(m_sGroup1, "reverse"), 1.0);
    ProcessBuffer();
    // Note: we cannot compare a golden buffer here, because the result depends
    // on the uses library version
}

TEST_F(EngineBufferE2ETest, RubberbandReverseTest) {
    // This test must not crash when changing to reverse while pitch is tweaked
    // Testing bug #1458263
    ControlObject::set(ConfigKey("[Master]", "keylock_engine"),
                       static_cast<double>(EngineBuffer::RUBBERBAND));
    ControlObject::set(ConfigKey(m_sGroup1, "pitch"), -1);
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ProcessBuffer();
    ControlObject::set(ConfigKey(m_sGroup1, "reverse"), 1.0);
    ProcessBuffer();
    // Note: we cannot compare a golden buffer here, because the result depends
    // on the uses library version
}

TEST_F(EngineBufferE2ETest, CueGotoAndStopTest) {
    // Be sure, that the Crossfade buffer is processed only once
    // Bug #1504838
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ProcessBuffer();
    ControlObject::set(ConfigKey(m_sGroup1, "cue_gotoandstop"), 1.0);
    ProcessBuffer();
    assertBufferMatchesReference(m_pEngineMaster->masterBuffer(),
                                 kProcessBufferSize, "CueGotoAndStopTest");
}

TEST_F(EngineBufferE2ETest, CueGotoAndPlayTest) {
    // Be sure, cue seek is not overwritten by quantization seek
    // Bug #1504503
    ControlObject::set(ConfigKey(m_sGroup1, "quantize"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup1, "cue_point"), 0.0);
    m_pChannel1->getEngineBuffer()->queueNewPlaypos(
            1000, EngineBuffer::SEEK_EXACT);
    ProcessBuffer();
    ControlObject::set(ConfigKey(m_sGroup1, "cue_gotoandplay"), 1.0);
    ProcessBuffer();
    assertBufferMatchesReference(m_pEngineMaster->masterBuffer(),
                                 kProcessBufferSize, "CueGotoAndPlayTest");
}

TEST_F(EngineBufferE2ETest, CueStartPlayTest) {
    // Be sure, cue seek is not overwritten by quantization seek
    // Bug #1504851
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ProcessBuffer();
    ControlObject::set(ConfigKey(m_sGroup1, "start_play"), 1.0);
    ProcessBuffer();
    assertBufferMatchesReference(m_pEngineMaster->masterBuffer(),
                                 kProcessBufferSize, "StartPlayTest");
}

TEST_F(EngineBufferE2ETest, CueGotoAndPlayDenon) {
    // Be sure, cue point is not moved
    // enable Denon mode Bug #1504934
    ControlObject::set(ConfigKey(m_sGroup1, "cue_mode"), 2.0); // CUE_MODE_DENON
    m_pChannel1->getEngineBuffer()->queueNewPlaypos(
            1000, EngineBuffer::SEEK_EXACT);
    ProcessBuffer();
    double cueBefore = ControlObject::get(ConfigKey(m_sGroup1, "cue_point"));
    ControlObject::set(ConfigKey(m_sGroup1, "cue_gotoandplay"), 1.0);
    ProcessBuffer();
    EXPECT_EQ(cueBefore, ControlObject::get(ConfigKey(m_sGroup1, "cue_point")));
}

TEST_F(EngineBufferTest, RateTempTest) {
    RateControl::setTemporaryRateChangeCoarseAmount(4);
    RateControl::setTemporaryRateChangeFineAmount(2);

    ControlObject::set(ConfigKey(m_sGroup1, "rate_dir"), 1);
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ProcessBuffer();
    EXPECT_EQ(1.0, m_pChannel1->getEngineBuffer()->m_speed_old);

    ProcessBuffer();

    ControlObject::set(ConfigKey(m_sGroup1, "rate_temp_up"), 1);
    ProcessBuffer();
    EXPECT_EQ(1.04, m_pChannel1->getEngineBuffer()->m_speed_old);
    ControlObject::set(ConfigKey(m_sGroup1, "rate_temp_up"), 0);

    ProcessBuffer();

    ControlObject::set(ConfigKey(m_sGroup1, "rate_temp_up_small"), 1);
    ProcessBuffer();
    EXPECT_EQ(1.02, m_pChannel1->getEngineBuffer()->m_speed_old);
    ControlObject::set(ConfigKey(m_sGroup1, "rate_temp_up_small"), 0);

    ProcessBuffer();

    ControlObject::set(ConfigKey(m_sGroup1, "rate_temp_down"), 1);
    ProcessBuffer();
    EXPECT_EQ(0.96, m_pChannel1->getEngineBuffer()->m_speed_old);
    ControlObject::set(ConfigKey(m_sGroup1, "rate_temp_down"), 0);

    ProcessBuffer();

    ControlObject::set(ConfigKey(m_sGroup1, "rate_temp_down_small"), 1);
    ProcessBuffer();
    EXPECT_EQ(0.98, m_pChannel1->getEngineBuffer()->m_speed_old);
    ControlObject::set(ConfigKey(m_sGroup1, "rate_temp_down_small"), 0);

    ControlObject::set(ConfigKey(m_sGroup1, "rate_dir"), -1);

    ProcessBuffer();

    ControlObject::set(ConfigKey(m_sGroup1, "rate_temp_up"), 1);
    ProcessBuffer();
    EXPECT_EQ(1.04, m_pChannel1->getEngineBuffer()->m_speed_old);
    ControlObject::set(ConfigKey(m_sGroup1, "rate_temp_up"), 0);

    ProcessBuffer();

    ControlObject::set(ConfigKey(m_sGroup1, "rate_temp_up_small"), 1);
    ProcessBuffer();
    EXPECT_EQ(1.02, m_pChannel1->getEngineBuffer()->m_speed_old);
    ControlObject::set(ConfigKey(m_sGroup1, "rate_temp_up_small"), 0);

    ProcessBuffer();

    ControlObject::set(ConfigKey(m_sGroup1, "rate_temp_down"), 1);
    ProcessBuffer();
    EXPECT_EQ(0.96, m_pChannel1->getEngineBuffer()->m_speed_old);
    ControlObject::set(ConfigKey(m_sGroup1, "rate_temp_down"), 0);

    ProcessBuffer();

    ControlObject::set(ConfigKey(m_sGroup1, "rate_temp_down_small"), 1);
    ProcessBuffer();
    EXPECT_EQ(0.98, m_pChannel1->getEngineBuffer()->m_speed_old);
    ControlObject::set(ConfigKey(m_sGroup1, "rate_temp_down_small"), 0);
}
