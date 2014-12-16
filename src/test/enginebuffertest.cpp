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
    ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm"))->set(128.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "keylock"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "pitch"))->set(0.5);
    ProcessBuffer();

    ControlObject::getControl(ConfigKey(m_sGroup1, "keylock"))->set(0.0);
    // We require a buffer process to see that the keylock state has changed.
    ProcessBuffer();
    ASSERT_EQ(0.0, ControlObject::getControl(ConfigKey(m_sGroup1, "pitch"))->get());
}

TEST_F(EngineBufferTest, TrackLoadResetsPitch) {
    // When a new track is loaded, the pitch value should be reset.
    ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm"))->set(128.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "pitch"))->set(0.5);
    ProcessBuffer();
    ASSERT_EQ(0.5, ControlObject::getControl(ConfigKey(m_sGroup1, "pitch"))->get());

    m_pChannel1->getEngineBuffer()->loadFakeTrack();
    ASSERT_EQ(0.0, ControlObject::getControl(ConfigKey(m_sGroup1, "pitch"))->get());
}
