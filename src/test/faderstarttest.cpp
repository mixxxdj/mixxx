// Tests for fader start feature implemented in BaseTrackPlayer.cpp

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "control/controlobject.h"
#include "test/mockedenginebackendtest.h"

class FaderStartTest : public MockedEngineBackendTest {};

// Test 1: Volume == 0 pauses Track
TEST_F(FaderStartTest, ZeroVolumeStopsPlay) {
    // Setting initial state of playing (paused) with non-zero volume
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup1, "volume"), 0.5);

    // Lowering volume to 0
    ControlObject::set(ConfigKey(m_sGroup1, "volume"), 0.0);
    ProcessBuffer();
    // ASSERTION: play should be 0.0
    EXPECT_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "play")));
}
