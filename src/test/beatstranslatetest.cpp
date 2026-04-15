#include <memory>

#include "test/mockedenginebackendtest.h"
#include "track/beats.h"

class BeatsTranslateTest : public MockedEngineBackendTest {
};

TEST_F(BeatsTranslateTest, SimpleTranslateMatch) {
    ControlObject::set(ConfigKey(m_sGroup1, QStringLiteral("quantize")), 0);
    ControlObject::set(ConfigKey(m_sGroup2, QStringLiteral("quantize")), 0);
    // Set up BeatGrids for decks 1 and 2.
    const auto bpm = mixxx::Bpm(60.0);
    constexpr auto firstBeat = mixxx::audio::kStartFramePos;
    auto grid1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), firstBeat, bpm);
    m_pTrack1->trySetBeats(grid1);
    ASSERT_DOUBLE_EQ(firstBeat.value(),
            grid1->findClosestBeat(mixxx::audio::kStartFramePos).value());

    auto grid2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), firstBeat, bpm);
    m_pTrack2->trySetBeats(grid2);
    ASSERT_DOUBLE_EQ(firstBeat.value(),
            grid2->findClosestBeat(mixxx::audio::kStartFramePos).value());

    // Seek deck 1 forward a bit.
    const auto seekPosition = mixxx::audio::FramePos{1111.0};
    m_pChannel1->getEngineBuffer()->seekAbs(seekPosition);
    ProcessBuffer();
    EXPECT_TRUE(m_pChannel1->getEngineBuffer()->getVisualPlayPos() > 0);

    // Make both decks playing.
    ControlObject::getControl(m_sGroup1, "play")->set(1.0);
    ControlObject::getControl(m_sGroup2, "play")->set(1.0);
    ProcessBuffer();
    // Manually set the "bpm" control... I would like to figure out why this
    // doesn't get set naturally, but this will do for now.
    auto pBpm1 = std::make_unique<ControlProxy>(m_sGroup1, "bpm");
    auto pBpm2 = std::make_unique<ControlProxy>(m_sGroup1, "bpm");
    pBpm1->set(bpm.value());
    pBpm2->set(bpm.value());
    ProcessBuffer();

    // Push the button on deck 2.
    auto pTranslateMatchAlignment = std::make_unique<ControlProxy>(
        m_sGroup2, "beats_translate_match_alignment");
    pTranslateMatchAlignment->set(1.0);
    ProcessBuffer();

    // Deck 1 is +seekPosition away from its closest beat (which is at 0).
    // Deck 2 was left at 0. We translated grid 2 so that it is also +seekPosition
    // away from its closest beat, so that beat should be at -seekPosition.
    mixxx::BeatsPointer pBeats = m_pTrack2->getBeats();
    EXPECT_FRAMEPOS_EQ(seekPosition * -1, pBeats->findClosestBeat(mixxx::audio::kStartFramePos));
}

TEST_F(BeatsTranslateTest, BeatsUndoTest) {
    const auto bpm60 = mixxx::Bpm(60.0);
    const auto origin123 = mixxx::audio::FramePos{123.0};
    auto grid = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, bpm60);
    m_pTrack1->trySetBeats(grid);
    EXPECT_FRAMEPOS_EQ(mixxx::audio::kStartFramePos, mixxx::audio::kStartFramePos);

    auto pBpm = std::make_unique<ControlProxy>(m_sGroup1, "bpm");
    auto pBeatsTranslateCurPos =
            std::make_unique<ControlProxy>(m_sGroup1, "beats_translate_curpos");
    auto pBeatsUndo = std::make_unique<ControlProxy>(m_sGroup1, "beats_undo_adjustment");
    pBpm->set(bpm60.value());
    m_pChannel1->getEngineBuffer()->seekAbs(origin123);
    ProcessBuffer(); // first process to schedule seek in a stopped deck
    ProcessBuffer(); // then seek
    // Beats undo ignores changes done in quick succession (ignore delay is 800 millis),
    // so after the each set of beat changes, the newest item in the undo stack
    QTest::qSleep(810);

    // Populate the beats_undo stack with 10 "beats_translate_curpos" actions at
    // different positions.
    // should be the state from before the sequence.
    // In order to have a before state, we translate the beats to 123.
    pBeatsTranslateCurPos->set(1.0);
    pBeatsTranslateCurPos->set(0.0);
    // Now sleep so only the steps of the sequence are recognized as quick actions.
    auto newSeekPosition = m_pChannel1->getEngineBuffer()->getExactPlayPos();
    EXPECT_FRAMEPOS_EQ(origin123, newSeekPosition);
    QTest::qSleep(810);

    for (int i = 0; i < 5; i++) {
        newSeekPosition += 5.0;
        m_pChannel1->getEngineBuffer()->seekAbs(newSeekPosition);
        ProcessBuffer(); // first process to schedule seek in a stopped deck
        ProcessBuffer(); // then seek
        pBeatsTranslateCurPos->set(1.0);
        pBeatsTranslateCurPos->set(0.0);
    }

    // We should be at 148 now
    newSeekPosition = m_pChannel1->getEngineBuffer()->getExactPlayPos();
    const auto pos148 = mixxx::audio::FramePos{148.0};
    EXPECT_FRAMEPOS_EQ(newSeekPosition, pos148);

    // Sleep to keep the current state in the undo stack.
    QTest::qSleep(810);

    for (int i = 0; i < 5; i++) {
        newSeekPosition += 5.0;
        m_pChannel1->getEngineBuffer()->seekAbs(newSeekPosition);
        ProcessBuffer(); // first process to schedule seek in a stopped deck
        ProcessBuffer(); // then seek
        pBeatsTranslateCurPos->set(1.0);
        pBeatsTranslateCurPos->set(0.0);
    }
    // After this quick sequence we're at frame 173.
    newSeekPosition = m_pChannel1->getEngineBuffer()->getExactPlayPos();
    EXPECT_FRAMEPOS_EQ(newSeekPosition, mixxx::audio::FramePos{173.0});

    pBeatsUndo->set(1.0);
    pBeatsUndo->set(0.0);
    mixxx::BeatsPointer pBeats = m_pTrack1->getBeats();
    EXPECT_FRAMEPOS_EQ(pos148, pBeats->findClosestBeat(mixxx::audio::kStartFramePos));

    // Undo once more to get back to 123.
    pBeatsUndo->set(1.0);
    pBeatsUndo->set(0.0);
    pBeats = m_pTrack1->getBeats();
    EXPECT_FRAMEPOS_EQ(origin123, pBeats->findClosestBeat(mixxx::audio::kStartFramePos));
}
