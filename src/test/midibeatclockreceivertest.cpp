#include <gtest/gtest.h>

#include "controllers/midi/midibeatclockreceiver.h"
#include "controllers/midi/midimessage.h"
#include "util/duration.h"

class MidiBeatClockReceiverTest : public testing::Test {};

TEST_F(MidiBeatClockReceiverTest, BpmDetection) {
    mixxx::MidiBeatClockReceiver beatClockReceiver;

    constexpr auto kQuarterNotesPerSecond = 100 * 24 / 60;
    constexpr auto kClockIntervalNanos =
            mixxx::Duration::fromNanos(1000000000 / kQuarterNotesPerSecond);

    for (int i = 0; i < 24; i++) {
        beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * i);
    }

    EXPECT_DOUBLE_EQ(100.0, beatClockReceiver.bpm().getValue());
}

TEST_F(MidiBeatClockReceiverTest, PhaseDetection) {
    mixxx::MidiBeatClockReceiver beatClockReceiver;
    EXPECT_DOUBLE_EQ(1.0, beatClockReceiver.beatDistance());

    constexpr auto kClockIntervalNanos = mixxx::Duration::fromNanos(25000000);
    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 1);
    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 2);
    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 3);
    EXPECT_DOUBLE_EQ(0.875, beatClockReceiver.beatDistance());

    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 4);
    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 5);
    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 6);
    EXPECT_DOUBLE_EQ(0.75, beatClockReceiver.beatDistance());

    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 7);
    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 8);
    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 9);
    EXPECT_DOUBLE_EQ(0.625, beatClockReceiver.beatDistance());

    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 10);
    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 11);
    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 12);
    EXPECT_DOUBLE_EQ(0.5, beatClockReceiver.beatDistance());

    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 13);
    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 14);
    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 15);
    EXPECT_DOUBLE_EQ(0.375, beatClockReceiver.beatDistance());

    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 16);
    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 17);
    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 18);
    EXPECT_DOUBLE_EQ(0.25, beatClockReceiver.beatDistance());

    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 19);
    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 20);
    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 21);
    EXPECT_DOUBLE_EQ(0.125, beatClockReceiver.beatDistance());

    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 22);
    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 23);
    beatClockReceiver.receive(MidiOpCode::MIDI_TIMING_CLK, kClockIntervalNanos * 24);
    EXPECT_DOUBLE_EQ(1.0, beatClockReceiver.beatDistance());
}
