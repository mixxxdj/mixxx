// Integration tests for EngineBuffer with the Bungee keylock engine.
//
// These tests exercise the REAL EngineBufferScaleBungee (not a mock scaler)
// so that engine-level state management — keylock enable/disable, engine
// switching, multi-buffer continuity — is validated end-to-end.
// setScalerForTest() is deliberately NOT called; m_bScalerOverride stays
// false so the normal keylock-engine selection code path is exercised.
//
// Higher-level regression coverage for keylock toggling with Bungee.

#include <gtest/gtest.h>

#include <cmath>

#include "control/controlobject.h"
#include "engine/enginebuffer.h"
#include "test/mockedenginebackendtest.h"
#include "test/signalpathtest.h"

// Helper: scan a span for NaN / Inf.
static bool spanHasInvalidSamples(std::span<const CSAMPLE> buf) {
    for (const CSAMPLE s : buf) {
        if (!std::isfinite(s)) {
            return true;
        }
    }
    return false;
}

// EngineBufferBungeeTest — fixture that selects the real Bungee scaler.
//
// Inherits BaseSignalPathTest (real signal path, real CachingReader, real
// ReadAheadManager) but does NOT call setScalerForTest(), so the scalers
// live in their natural EngineBuffer slots.
class EngineBufferBungeeTest : public BaseSignalPathTest {
  protected:
    static const QString kAppGroup;

    void SetUp() override {
        BaseSignalPathTest::SetUp();
        // Load a fake 128-BPM stereo track on deck 1 and let it play.
        m_pTrack1 = m_pMixerDeck1->loadFakeTrack(false, 128.0);
        ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
        ProcessBuffer();
    }

    void selectEngine(EngineBuffer::KeylockEngine eng) {
        ControlObject::set(ConfigKey(kAppGroup, QStringLiteral("keylock_engine")),
                static_cast<double>(eng));
    }

    void setKeylock(bool on) {
        ControlObject::set(ConfigKey(m_sGroup1, "keylock"), on ? 1.0 : 0.0);
    }

    // Drive the engine for n buffers; return true iff all output was finite.
    bool processFinite(int n) {
        for (int i = 0; i < n; ++i) {
            ProcessBuffer();
            if (spanHasInvalidSamples(m_pEngineMixer->getMainBuffer())) {
                return false;
            }
        }
        return true;
    }

    TrackPointer m_pTrack1;
};

// static
const QString EngineBufferBungeeTest::kAppGroup = QStringLiteral("[App]");

// When Bungee is the selected keylock engine and keylock is
// enabled, m_pScaleKeylock must point at m_pScaleBungee and m_pScale must
// follow.
TEST_F(EngineBufferBungeeTest, BungeeEngineSelected) {
    selectEngine(EngineBuffer::KeylockEngine::Bungee);
    ProcessBuffer();

    EngineBuffer* pEB = m_pChannel1->getEngineBuffer();
    EXPECT_EQ(pEB->m_pScaleBungee, pEB->m_pScaleKeylock);

    setKeylock(true);
    ProcessBuffer();
    EXPECT_EQ(pEB->m_pScaleBungee, pEB->m_pScale);

    // Several more clean buffers while keylock is on.
    EXPECT_TRUE(processFinite(5));
}

// Rapidly toggling keylock on/off while Bungee is the active
// engine must not produce NaN/Inf output or crash.
TEST_F(EngineBufferBungeeTest, BungeeKeylockToggleDoesNotCrash) {
    selectEngine(EngineBuffer::KeylockEngine::Bungee);
    ProcessBuffer();

    for (int i = 0; i < 8; ++i) {
        setKeylock(i % 2 == 0);
        EXPECT_TRUE(processFinite(2))
                << "Invalid audio detected at toggle iteration " << i;
    }

    // Stabilize with keylock on.
    setKeylock(true);
    EXPECT_TRUE(processFinite(6));
}

// Switching the keylock engine from SoundTouch to Bungee and back
// while playing must keep m_pScaleKeylock consistent and produce clean audio.
TEST_F(EngineBufferBungeeTest, BungeeKeylockEngineSwitch) {
    // Start with SoundTouch.
    selectEngine(EngineBuffer::KeylockEngine::SoundTouch);
    setKeylock(true);
    EXPECT_TRUE(processFinite(4));

    EngineBuffer* pEB = m_pChannel1->getEngineBuffer();
    EXPECT_NE(pEB->m_pScaleBungee, pEB->m_pScaleKeylock);

    // Switch to Bungee mid-play.
    selectEngine(EngineBuffer::KeylockEngine::Bungee);
    EXPECT_TRUE(processFinite(1));
    EXPECT_EQ(pEB->m_pScaleBungee, pEB->m_pScaleKeylock);
    EXPECT_TRUE(processFinite(4));

    // Switch back to SoundTouch.
    selectEngine(EngineBuffer::KeylockEngine::SoundTouch);
    EXPECT_TRUE(processFinite(1));
    EXPECT_NE(pEB->m_pScaleBungee, pEB->m_pScaleKeylock);
    EXPECT_TRUE(processFinite(4));
}
