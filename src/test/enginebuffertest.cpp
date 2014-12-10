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
    ControlObject::set(ConfigKey(m_sGroup1, "file_bpm"), 128.0);
    ControlObject::set(ConfigKey(m_sGroup1, "keylock"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup1, "pitch"),0.5);
    ProcessBuffer();

    ControlObject::set(ConfigKey(m_sGroup1, "keylock"), 0.0);
    // We require a buffer process to see that the keylock state has changed.
    ProcessBuffer();
    ASSERT_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "pitch")));
}

TEST_F(EngineBufferTest, TrackLoadResetsPitch) {
    // When a new track is loaded, the pitch value should be reset.
    ControlObject::set(ConfigKey(m_sGroup1, "file_bpm"), 128.0);
    ControlObject::set(ConfigKey(m_sGroup1, "pitch"), 0.5);
    ProcessBuffer();
    ASSERT_EQ(0.5, ControlObject::get(ConfigKey(m_sGroup1, "pitch")));

    m_pChannel1->getEngineBuffer()->loadFakeTrack();
    ASSERT_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "pitch")));
}
