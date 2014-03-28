#include <stdint.h>

#include "test/mixxxtest.h"
#include "controllers/learningutils.h"

std::ostream& operator<<(std::ostream& stream, const QPair<MidiKey, MidiOptions>& key_options) {
    stream << key_options.first.key << key_options.second.all;
    return stream;
}

class LearningUtilsTest : public MixxxTest {
  protected:
    void addMessage(unsigned char status, unsigned char control, unsigned char value) {
        m_messages.append(qMakePair(MidiKey(status, control), value));
    }

    QList<QPair<MidiKey, unsigned char> > m_messages;
};

TEST_F(LearningUtilsTest, NoteOnButton) {
    // Status: 0x91, Control: 0x10
    addMessage(MIDI_NOTE_ON | 0x01, 0x10, 0x7F);
    addMessage(MIDI_NOTE_ON | 0x01, 0x10, 0x00);

    MidiKeyAndOptionsList mappings =
            LearningUtils::guessMidiInputMappings(m_messages);

    ASSERT_EQ(1, mappings.size());
    EXPECT_EQ(qMakePair(MidiKey(MIDI_NOTE_ON | 0x01, 0x10), MidiOptions()),
              mappings.first());
}

TEST_F(LearningUtilsTest, NoteOnNoteOffButton) {
    // Status: 0x91, Control: 0x10
    addMessage(MIDI_NOTE_ON | 0x01, 0x10, 0x7F);
    addMessage(MIDI_NOTE_OFF | 0x01, 0x10, 0x00);

    MidiKeyAndOptionsList mappings =
            LearningUtils::guessMidiInputMappings(m_messages);

    ASSERT_EQ(2, mappings.size());
    EXPECT_EQ(qMakePair(MidiKey(MIDI_NOTE_ON | 0x01, 0x10), MidiOptions()),
              mappings.at(0));
    EXPECT_EQ(qMakePair(MidiKey(MIDI_NOTE_OFF | 0x01, 0x10), MidiOptions()),
              mappings.at(1));
}

TEST_F(LearningUtilsTest, CC7BitKnob) {
    // Standard CC 7-bit knobs show up as a MIDI_CC message, single channel,
    // single control and a variety of values in the range of 0x00 to 0x7F.
    // Status: 0x81 Control: 0x01
    addMessage(MIDI_CC | 0x01, 0x10, 0x7F);
    addMessage(MIDI_CC | 0x01, 0x10, 0x70);
    addMessage(MIDI_CC | 0x01, 0x10, 0x60);
    addMessage(MIDI_CC | 0x01, 0x10, 0x50);
    addMessage(MIDI_CC | 0x01, 0x10, 0x60);
    addMessage(MIDI_CC | 0x01, 0x10, 0x50);
    addMessage(MIDI_CC | 0x01, 0x10, 0x40);

    MidiKeyAndOptionsList mappings =
            LearningUtils::guessMidiInputMappings(m_messages);

    ASSERT_EQ(1, mappings.size());
    EXPECT_EQ(qMakePair(MidiKey(MIDI_CC | 0x01, 0x10), MidiOptions()),
              mappings.at(0));
}

TEST_F(LearningUtilsTest, CC7BitTicker) {
    // A CC 7-bit ticker (select knob, jog wheel, etc.) shows up as a MIDI_CC
    // message, single channel, single control and a variety of values in two's
    // complement. We detect this by looking at jumps across the boundary of
    // 0x7F to 0x00 so the user has to go forward and backward.
    // Status: 0x81 Control: 0x10
    addMessage(MIDI_CC | 0x01, 0x10, 0x7F);
    addMessage(MIDI_CC | 0x01, 0x10, 0x01);
    addMessage(MIDI_CC | 0x01, 0x10, 0x02);
    addMessage(MIDI_CC | 0x01, 0x10, 0x05);
    addMessage(MIDI_CC | 0x01, 0x10, 0x7E);
    addMessage(MIDI_CC | 0x01, 0x10, 0x7C);
    addMessage(MIDI_CC | 0x01, 0x10, 0x70);

    MidiKeyAndOptionsList mappings =
            LearningUtils::guessMidiInputMappings(m_messages);

    ASSERT_EQ(1, mappings.size());
    MidiOptions options;
    options.selectknob = true;
    EXPECT_EQ(qMakePair(MidiKey(MIDI_CC | 0x01, 0x10), options),
              mappings.at(0));
}

TEST_F(LearningUtilsTest, Spread64Ticker) {
    // A Spread64 ticker (select knob, jog wheel, etc.) shows up as a MIDI_CC
    // message, single channel, single control and a variety of values centered
    // around 0x40 but never including that value (since 0x40 means "not moving")
    addMessage(MIDI_CC | 0x01, 0x10, 0x41);
    addMessage(MIDI_CC | 0x01, 0x10, 0x41);
    addMessage(MIDI_CC | 0x01, 0x10, 0x42);
    addMessage(MIDI_CC | 0x01, 0x10, 0x41);
    addMessage(MIDI_CC | 0x01, 0x10, 0x3F);
    addMessage(MIDI_CC | 0x01, 0x10, 0x3F);
    addMessage(MIDI_CC | 0x01, 0x10, 0x3F);

    MidiKeyAndOptionsList mappings =
            LearningUtils::guessMidiInputMappings(m_messages);

    ASSERT_EQ(1, mappings.size());
    MidiOptions options;
    options.spread64 = true;
    EXPECT_EQ(qMakePair(MidiKey(MIDI_CC | 0x01, 0x10), options),
              mappings.at(0));
}

TEST_F(LearningUtilsTest, CC7BitTicker_SingleDirection) {
    // A CC 7-bit ticker (select knob, jog wheel, etc.) shows up as a MIDI_CC
    // message, single channel, single control and a variety of values in two's
    // complement. It's harder to detect when the user does not change direction
    // because we do not see 0x00 boundary crossings that are tell-tale signs of
    // two's complement. If we see repeat 0x01 or 0x7F values then we interpret
    // those as being a ticker as well.

    // Status: 0x81 Control: 0x10
    addMessage(MIDI_CC | 0x01, 0x10, 0x01);
    addMessage(MIDI_CC | 0x01, 0x10, 0x02);
    addMessage(MIDI_CC | 0x01, 0x10, 0x01);
    addMessage(MIDI_CC | 0x01, 0x10, 0x03);
    addMessage(MIDI_CC | 0x01, 0x10, 0x01);

    MidiOptions options;
    options.selectknob = true;

    MidiKeyAndOptionsList mappings =
            LearningUtils::guessMidiInputMappings(m_messages);
    ASSERT_EQ(1, mappings.size());
    EXPECT_EQ(qMakePair(MidiKey(MIDI_CC | 0x01, 0x10), options),
              mappings.at(0));

    mappings.clear();

    // Status: 0x81 Control: 0x10
    addMessage(MIDI_CC | 0x01, 0x10, 0x7F);
    addMessage(MIDI_CC | 0x01, 0x10, 0x7E);
    addMessage(MIDI_CC | 0x01, 0x10, 0x7F);
    addMessage(MIDI_CC | 0x01, 0x10, 0x7C);

    mappings = LearningUtils::guessMidiInputMappings(m_messages);
    ASSERT_EQ(1, mappings.size());
    EXPECT_EQ(qMakePair(MidiKey(MIDI_CC | 0x01, 0x10), options),
              mappings.at(0));
}

TEST_F(LearningUtilsTest, SingleMessageSwitchMode) {
    // If we only get one NOTE_ON message that is either 0x7F or 0x00 then we
    // assume this is a binary toggle switch.

    // Status: 0x91 Control: 0x10
    addMessage(MIDI_NOTE_ON | 0x01, 0x10, 0x7F);

    MidiKeyAndOptionsList mappings =
            LearningUtils::guessMidiInputMappings(m_messages);

    ASSERT_EQ(1, mappings.size());
    MidiOptions options;
    options.sw = true;
    EXPECT_EQ(qMakePair(MidiKey(MIDI_NOTE_ON | 0x01, 0x10), options),
              mappings.at(0));

    m_messages.clear();

    // Status: 0x91 Control: 0x10
    addMessage(MIDI_NOTE_ON | 0x01, 0x10, 0x00);

    mappings = LearningUtils::guessMidiInputMappings(m_messages);

    ASSERT_EQ(1, mappings.size());
    EXPECT_EQ(qMakePair(MidiKey(MIDI_NOTE_ON | 0x01, 0x10), options),
              mappings.at(0));
}

TEST_F(LearningUtilsTest, MultipleControlsUnrecognized_BindsFirst) {
    // Status 0x91, Control 0x10
    addMessage(MIDI_NOTE_ON | 0x01, 0x10, 0x7F);
    addMessage(MIDI_NOTE_ON | 0x01, 0x10, 0x00);

    // Status 0x91, Control 0x11
    addMessage(MIDI_NOTE_ON | 0x01, 0x11, 0x7F);
    addMessage(MIDI_NOTE_ON | 0x01, 0x11, 0x00);

    MidiKeyAndOptionsList mappings =
            LearningUtils::guessMidiInputMappings(m_messages);
    ASSERT_EQ(1, mappings.size());
    EXPECT_EQ(qMakePair(MidiKey(MIDI_NOTE_ON | 0x01, 0x10), MidiOptions()),
              mappings.first());
}

TEST_F(LearningUtilsTest, MultipleChannelsUnrecognized_BindsFirst) {
    // Status 0x91, Control 0x10
    addMessage(MIDI_NOTE_ON | 0x01, 0x10, 0x7F);
    addMessage(MIDI_NOTE_ON | 0x01, 0x10, 0x00);

    // Status 0x91, Control 0x11
    addMessage(MIDI_NOTE_ON | 0x02, 0x10, 0x7F);
    addMessage(MIDI_NOTE_ON | 0x02, 0x10, 0x00);

    MidiKeyAndOptionsList mappings =
            LearningUtils::guessMidiInputMappings(m_messages);
    ASSERT_EQ(1, mappings.size());
    EXPECT_EQ(qMakePair(MidiKey(MIDI_NOTE_ON | 0x01, 0x10), MidiOptions()),
              mappings.first());
}
