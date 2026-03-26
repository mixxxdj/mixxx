// Tests for fader start feature (BaseTrackPlayerImpl::slotVolumeChanged)

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "control/controlobject.h"
#include "test/mockedenginebackendtest.h"

class FaderStartTest : public MockedEngineBackendTest {};

// Test 1: Volume changed to 0 pauses Track
TEST_F(FaderStartTest, ZeroVolumeStopsPlay) {
    // Initial State:playing, with non-zero volume
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup1, "volume"), 0.5);

    // Lowering volume to 0
    ControlObject::set(ConfigKey(m_sGroup1, "volume"), 0.0);
    // Processing buffer
    ProcessBuffer();
    // ASSERTION: play should be 0.0
    EXPECT_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "play")));
}

// Test 2: Volume raised above 0 plays Track
TEST_F(FaderStartTest, RaisedVolumeStartsPlay) {
    // Initial State: paused with 0 volume
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 0.0);
    ControlObject::set(ConfigKey(m_sGroup1, "volume"), 0.0);

    // Raising volume to 0.5
    ControlObject::set(ConfigKey(m_sGroup1, "volume"), 0.5);
    // Processing buffer
    ProcessBuffer();
    // ASSERTION: play should be 1.0
    EXPECT_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup1, "play")));
}
