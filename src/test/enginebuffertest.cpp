// Tests for enginebuffer.cpp

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QtDebug>

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
    config()->set(ConfigKey("[Controls]","SpeedAutoReset"), ConfigValue(1));
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

    // With Soundtouch, the scaler should still be the keylock scaler
    ControlObject::set(ConfigKey("[Master]", "keylock_engine"),
                       static_cast<double>(EngineBuffer::SOUNDTOUCH));
    ProcessBuffer();
    EXPECT_EQ(m_pMockScaleKeylock1, m_pChannel1->getEngineBuffer()->m_pScale);

    // With Rubberband, the scaler should be linear
    ControlObject::set(ConfigKey("[Master]", "keylock_engine"),
                       static_cast<double>(EngineBuffer::RUBBERBAND));
    ProcessBuffer();
    EXPECT_EQ(m_pMockScaleVinyl1, m_pChannel1->getEngineBuffer()->m_pScale);
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
