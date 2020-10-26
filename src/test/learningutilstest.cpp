#include <stdint.h>

#include "control/controlobject.h"
#include "controllers/learningutils.h"
#include "test/mixxxtest.h"
#include "util/memory.h"

std::ostream& operator<<(std::ostream& stream, const MidiInputMapping& mapping) {
    stream << mapping.key.key << mapping.options.all;
    return stream;
}

class LearningUtilsTest : public MixxxTest {
  protected:
    void addMessage(unsigned char status, unsigned char control, unsigned char value) {
        m_messages.append(qMakePair(MidiKey(status, control), value));
    }

    /// Check if mapping in present in mapping list.
    /// Similar to MidiInputMappings::contains(const MidiInputMapping&), but
    /// does not compare the description.
    bool containsMapping(const MidiInputMappings& haystack, const MidiInputMapping& needle) {
        for (const MidiInputMapping& mapping : haystack) {
            if (mapping.key == needle.key &&
                    mapping.options == needle.options &&
                    mapping.control == needle.control) {
                return true;
            }
        }
        return false;
    }

    QList<QPair<MidiKey, unsigned char> > m_messages;
};

TEST_F(LearningUtilsTest, NoteOnButton) {
    // Status: 0x91, Control: 0x10
    addMessage(MIDI_NOTE_ON | 0x01, 0x10, 0x7F);
    addMessage(MIDI_NOTE_ON | 0x01, 0x10, 0x00);

    ConfigKey control("[Test]", "SomeControl");
    MidiInputMappings mappings =
            LearningUtils::guessMidiInputMappings(control, m_messages);

    ASSERT_EQ(1, mappings.size());
    EXPECT_EQ(MidiInputMapping(MidiKey(MIDI_NOTE_ON | 0x01, 0x10),
                      MidiOptions(),
                      control,
                      mappings.first().description),
            mappings.first());
}

TEST_F(LearningUtilsTest, NoteOnNoteOffButton) {
    // Status: 0x91, Control: 0x10
    addMessage(MIDI_NOTE_ON | 0x01, 0x10, 0x7F);
    addMessage(MIDI_NOTE_OFF | 0x01, 0x10, 0x00);

    ConfigKey control("[Test]", "SomeControl");
    MidiInputMappings mappings =
            LearningUtils::guessMidiInputMappings(control, m_messages);

    ASSERT_EQ(2, mappings.size());
    EXPECT_EQ(MidiInputMapping(MidiKey(MIDI_NOTE_ON | 0x01, 0x10),
                      MidiOptions(),
                      control,
                      mappings.at(0).description),
            mappings.at(0));
    EXPECT_EQ(MidiInputMapping(MidiKey(MIDI_NOTE_OFF | 0x01, 0x10),
                      MidiOptions(),
                      control,
                      mappings.at(1).description),
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

    ConfigKey control("[Test]", "SomeControl");
    MidiInputMappings mappings =
            LearningUtils::guessMidiInputMappings(control, m_messages);

    ASSERT_EQ(1, mappings.size());
    EXPECT_EQ(MidiInputMapping(MidiKey(MIDI_CC | 0x01, 0x10),
                      MidiOptions(),
                      control,
                      mappings.first().description),
            mappings.first());
}

TEST_F(LearningUtilsTest, CC7BitKnob_CenterPointButton_NoteOn) {
    // This style of 7-bit CC knob is found on controllers like the
    // VCI-100. They do not emit a center-point CC message with value 0x40 and
    // instead emit a NOTE_ON (0x7F, 0x00) pair for the center point. This
    // causes Mixxx to confuse these knobs for buttons and also cause them to
    // never truly reach their center point. We map the button to the
    // _set_default control if it exists.

    // Starting outside the center point.
    addMessage(MIDI_CC | 0x01, 0x10, 0x7F);
    addMessage(MIDI_CC | 0x01, 0x10, 0x70);
    addMessage(MIDI_CC | 0x01, 0x10, 0x60);
    addMessage(MIDI_CC | 0x01, 0x10, 0x50);
    addMessage(MIDI_CC | 0x01, 0x10, 0x41);
    addMessage(MIDI_NOTE_ON | 0x01, 0xE0, 0x7F);
    addMessage(MIDI_NOTE_ON | 0x01, 0xE0, 0x00);
    addMessage(MIDI_CC | 0x01, 0x10, 0x3F);
    addMessage(MIDI_CC | 0x01, 0x10, 0x30);
    addMessage(MIDI_CC | 0x01, 0x10, 0x20);
    addMessage(MIDI_CC | 0x01, 0x10, 0x10);
    addMessage(MIDI_CC | 0x01, 0x10, 0x00);

    ConfigKey control("[Test]", "SomeControl");
    // This message recognizer checks for the existence of a _reset control.
    ConfigKey resetControl("[Test]", "SomeControl_set_default");
    auto pResetControl = std::make_unique<ControlObject>(resetControl);
    MidiInputMappings mappings =
            LearningUtils::guessMidiInputMappings(control, m_messages);

    ASSERT_EQ(2, mappings.size());
    EXPECT_TRUE(containsMapping(mappings,
            MidiInputMapping(
                    MidiKey(MIDI_CC | 0x01, 0x10), MidiOptions(), control)));
    EXPECT_TRUE(containsMapping(mappings,
            MidiInputMapping(MidiKey(MIDI_NOTE_ON | 0x01, 0xE0),
                    MidiOptions(),
                    resetControl)));

    m_messages.clear();

    // Starting on the center point the VCI-100 only sends a NOTE_ON with value
    // 0x00 to indicate we started moving but are leaving the center point.
    addMessage(MIDI_NOTE_ON | 0x01, 0xE0, 0x00);
    addMessage(MIDI_CC | 0x01, 0x10, 0x3F);
    addMessage(MIDI_CC | 0x01, 0x10, 0x35);
    addMessage(MIDI_CC | 0x01, 0x10, 0x30);
    addMessage(MIDI_CC | 0x01, 0x10, 0x25);
    addMessage(MIDI_CC | 0x01, 0x10, 0x20);
    addMessage(MIDI_CC | 0x01, 0x10, 0x15);
    addMessage(MIDI_CC | 0x01, 0x10, 0x10);
    addMessage(MIDI_CC | 0x01, 0x10, 0x05);
    addMessage(MIDI_CC | 0x01, 0x10, 0x00);

    ASSERT_EQ(2, mappings.size());
    EXPECT_TRUE(containsMapping(mappings,
            MidiInputMapping(
                    MidiKey(MIDI_CC | 0x01, 0x10), MidiOptions(), control)));
    EXPECT_TRUE(containsMapping(mappings,
            MidiInputMapping(MidiKey(MIDI_NOTE_ON | 0x01, 0xE0),
                    MidiOptions(),
                    resetControl)));
}

TEST_F(LearningUtilsTest, CC14BitKnob_MSBFirst) {
    // A CC 14-bit knob with the MSB as the first message. We treat streams of
    // single-channel, CC opcode, 2 control, equal size message streams as
    // 14-bit CC knobs. The order of the LSB and MSB does not matter to us.

    addMessage(MIDI_CC | 0x01, 0x11, 0x00);
    addMessage(MIDI_CC | 0x01, 0x10, 0x00);
    addMessage(MIDI_CC | 0x01, 0x11, 0x00);
    addMessage(MIDI_CC | 0x01, 0x10, 0x40);
    addMessage(MIDI_CC | 0x01, 0x11, 0x00);
    addMessage(MIDI_CC | 0x01, 0x10, 0x7F);
    addMessage(MIDI_CC | 0x01, 0x11, 0x01);
    addMessage(MIDI_CC | 0x01, 0x10, 0x00);
    addMessage(MIDI_CC | 0x01, 0x11, 0x01);
    addMessage(MIDI_CC | 0x01, 0x10, 0x40);
    addMessage(MIDI_CC | 0x01, 0x11, 0x01);
    addMessage(MIDI_CC | 0x01, 0x10, 0x7F);
    addMessage(MIDI_CC | 0x01, 0x11, 0x02);
    addMessage(MIDI_CC | 0x01, 0x10, 0x00);
    addMessage(MIDI_CC | 0x01, 0x11, 0x02);
    addMessage(MIDI_CC | 0x01, 0x10, 0x40);
    addMessage(MIDI_CC | 0x01, 0x11, 0x02);
    addMessage(MIDI_CC | 0x01, 0x10, 0x7F);
    addMessage(MIDI_CC | 0x01, 0x11, 0x03);
    addMessage(MIDI_CC | 0x01, 0x10, 0x00);
    addMessage(MIDI_CC | 0x01, 0x11, 0x03);
    addMessage(MIDI_CC | 0x01, 0x10, 0x40);
    addMessage(MIDI_CC | 0x01, 0x11, 0x03);
    addMessage(MIDI_CC | 0x01, 0x10, 0x7F);

    ConfigKey control("[Test]", "SomeControl");
    MidiInputMappings mappings =
            LearningUtils::guessMidiInputMappings(control, m_messages);

    ASSERT_EQ(2, mappings.size());
    MidiOptions lsb_option;
    lsb_option.fourteen_bit_lsb = true;
    EXPECT_TRUE(containsMapping(mappings,
            MidiInputMapping(
                    MidiKey(MIDI_CC | 0x01, 0x10), lsb_option, control)));
    MidiOptions msb_option;
    msb_option.fourteen_bit_msb = true;
    EXPECT_TRUE(containsMapping(mappings,
            MidiInputMapping(
                    MidiKey(MIDI_CC | 0x01, 0x11), msb_option, control)));
}

TEST_F(LearningUtilsTest, CC14BitKnob_LSBFirst) {
    // A CC 14-bit knob with the LSB as the first message. We treat streams of
    // single-channel, CC opcode, 2 control, equal size message streams as
    // 14-bit CC knobs. The order of the LSB and MSB does not matter to us.

    addMessage(MIDI_CC | 0x01, 0x10, 0x00);
    addMessage(MIDI_CC | 0x01, 0x11, 0x00);
    addMessage(MIDI_CC | 0x01, 0x10, 0x40);
    addMessage(MIDI_CC | 0x01, 0x11, 0x00);
    addMessage(MIDI_CC | 0x01, 0x10, 0x7F);
    addMessage(MIDI_CC | 0x01, 0x11, 0x00);
    addMessage(MIDI_CC | 0x01, 0x10, 0x00);
    addMessage(MIDI_CC | 0x01, 0x11, 0x01);
    addMessage(MIDI_CC | 0x01, 0x10, 0x40);
    addMessage(MIDI_CC | 0x01, 0x11, 0x01);
    addMessage(MIDI_CC | 0x01, 0x10, 0x7F);
    addMessage(MIDI_CC | 0x01, 0x11, 0x01);
    addMessage(MIDI_CC | 0x01, 0x10, 0x00);
    addMessage(MIDI_CC | 0x01, 0x11, 0x02);
    addMessage(MIDI_CC | 0x01, 0x10, 0x40);
    addMessage(MIDI_CC | 0x01, 0x11, 0x02);
    addMessage(MIDI_CC | 0x01, 0x10, 0x7F);
    addMessage(MIDI_CC | 0x01, 0x11, 0x02);
    addMessage(MIDI_CC | 0x01, 0x10, 0x00);
    addMessage(MIDI_CC | 0x01, 0x11, 0x03);
    addMessage(MIDI_CC | 0x01, 0x10, 0x40);
    addMessage(MIDI_CC | 0x01, 0x11, 0x03);
    addMessage(MIDI_CC | 0x01, 0x10, 0x7F);
    addMessage(MIDI_CC | 0x01, 0x11, 0x03);

    ConfigKey control("[Test]", "SomeControl");
    MidiInputMappings mappings =
            LearningUtils::guessMidiInputMappings(control, m_messages);

    ASSERT_EQ(2, mappings.size());
    MidiOptions lsb_option;
    lsb_option.fourteen_bit_lsb = true;
    EXPECT_TRUE(containsMapping(mappings,
            MidiInputMapping(
                    MidiKey(MIDI_CC | 0x01, 0x10), lsb_option, control)));
    MidiOptions msb_option;
    msb_option.fourteen_bit_msb = true;
    EXPECT_TRUE(containsMapping(mappings,
            MidiInputMapping(
                    MidiKey(MIDI_CC | 0x01, 0x11), msb_option, control)));
}

TEST_F(LearningUtilsTest, CC7BitKnob_ConfusableForCC7BitTicker_Zeroes) {
    // Make sure that 0x00 is a tell that we are not a 2's complement select
    // knob.
    addMessage(MIDI_CC | 0x01, 0x10, 0x01);
    addMessage(MIDI_CC | 0x01, 0x10, 0x02);
    addMessage(MIDI_CC | 0x01, 0x10, 0x03);
    addMessage(MIDI_CC | 0x01, 0x10, 0x02);
    addMessage(MIDI_CC | 0x01, 0x10, 0x01);

    ConfigKey control("[Test]", "SomeControl");
    MidiInputMappings mappings =
            LearningUtils::guessMidiInputMappings(control, m_messages);

    MidiOptions options;
    options.selectknob = true;
    ASSERT_EQ(1, mappings.size());
    EXPECT_EQ(MidiInputMapping(MidiKey(MIDI_CC | 0x01, 0x10),
                      options,
                      control,
                      mappings.first().description),
            mappings.first());

    m_messages.clear();

    addMessage(MIDI_CC | 0x01, 0x10, 0x00);
    addMessage(MIDI_CC | 0x01, 0x10, 0x01);
    addMessage(MIDI_CC | 0x01, 0x10, 0x02);
    addMessage(MIDI_CC | 0x01, 0x10, 0x03);
    addMessage(MIDI_CC | 0x01, 0x10, 0x02);
    addMessage(MIDI_CC | 0x01, 0x10, 0x01);

    mappings = LearningUtils::guessMidiInputMappings(control, m_messages);

    ASSERT_EQ(1, mappings.size());
    EXPECT_EQ(MidiInputMapping(MidiKey(MIDI_CC | 0x01, 0x10),
                      MidiOptions(),
                      control,
                      mappings.first().description),
            mappings.first());
}

TEST_F(LearningUtilsTest, CC7BitKnob_ConfusableForCC7BitTicker_ZeroIncluded) {
    // Moving a CC knob through its range multiple times is confusable for
    // select knobs in some cases. More than 8 distinct values tell us this is
    // not a select knob.
    addMessage(MIDI_CC | 0x01, 0x10, 0x7F);
    addMessage(MIDI_CC | 0x01, 0x10, 0x60);
    addMessage(MIDI_CC | 0x01, 0x10, 0x50);
    addMessage(MIDI_CC | 0x01, 0x10, 0x40);
    addMessage(MIDI_CC | 0x01, 0x10, 0x30);
    addMessage(MIDI_CC | 0x01, 0x10, 0x20);
    addMessage(MIDI_CC | 0x01, 0x10, 0x10);
    addMessage(MIDI_CC | 0x01, 0x10, 0x01);
    // Zero tells us this is not a select knob.
    addMessage(MIDI_CC | 0x01, 0x10, 0x00);
    addMessage(MIDI_CC | 0x01, 0x10, 0x01);
    addMessage(MIDI_CC | 0x01, 0x10, 0x10);
    addMessage(MIDI_CC | 0x01, 0x10, 0x20);
    addMessage(MIDI_CC | 0x01, 0x10, 0x30);
    addMessage(MIDI_CC | 0x01, 0x10, 0x40);
    addMessage(MIDI_CC | 0x01, 0x10, 0x50);
    addMessage(MIDI_CC | 0x01, 0x10, 0x60);
    addMessage(MIDI_CC | 0x01, 0x10, 0x7F);

    ConfigKey control("[Test]", "SomeControl");
    MidiInputMappings mappings =
            LearningUtils::guessMidiInputMappings(control, m_messages);

    ASSERT_EQ(1, mappings.size());
    EXPECT_EQ(MidiInputMapping(MidiKey(MIDI_CC | 0x01, 0x10),
                      MidiOptions(),
                      control,
                      mappings.first().description),
            mappings.first());
}

TEST_F(LearningUtilsTest, CC7BitKnob_ConfusableForCC7BitTicker) {
    // Moving a CC knob through its range multiple times is confusable for
    // select knobs when a 0x7F is repeated. More than 8 distinct values tells
    // us this is not a select knob.
    addMessage(MIDI_CC | 0x01, 0x10, 0x7F);
    addMessage(MIDI_CC | 0x01, 0x10, 0x75);
    addMessage(MIDI_CC | 0x01, 0x10, 0x70);
    addMessage(MIDI_CC | 0x01, 0x10, 0x65);
    addMessage(MIDI_CC | 0x01, 0x10, 0x60);
    addMessage(MIDI_CC | 0x01, 0x10, 0x55);
    addMessage(MIDI_CC | 0x01, 0x10, 0x50);
    addMessage(MIDI_CC | 0x01, 0x10, 0x45);
    addMessage(MIDI_CC | 0x01, 0x10, 0x50);
    addMessage(MIDI_CC | 0x01, 0x10, 0x60);
    addMessage(MIDI_CC | 0x01, 0x10, 0x75);
    addMessage(MIDI_CC | 0x01, 0x10, 0x7F);

    ConfigKey control("[Test]", "SomeControl");
    MidiInputMappings mappings =
            LearningUtils::guessMidiInputMappings(control, m_messages);

    ASSERT_EQ(1, mappings.size());
    EXPECT_EQ(MidiInputMapping(MidiKey(MIDI_CC | 0x01, 0x10),
                      MidiOptions(),
                      control,
                      mappings.first().description),
            mappings.first());
}

TEST_F(LearningUtilsTest,
        CC7BitKnob_ConfusableForSpread64Ticker_StartAndStopOn41) {
    // Moving a CC knob through its range multiple times is confusable for
    // Spread64 select knobs when a 0x41 or 0x3F is repeated. If we start and
    // stop on 0x41 (and don't pass through 0x40) then this can set off Spread64
    // detection. We require <8 distinct values for Spread64 detection to
    // prevent this.
    addMessage(MIDI_CC | 0x01, 0x10, 0x41);
    addMessage(MIDI_CC | 0x01, 0x10, 0x45);
    addMessage(MIDI_CC | 0x01, 0x10, 0x50);
    addMessage(MIDI_CC | 0x01, 0x10, 0x55);
    addMessage(MIDI_CC | 0x01, 0x10, 0x60);
    addMessage(MIDI_CC | 0x01, 0x10, 0x65);
    addMessage(MIDI_CC | 0x01, 0x10, 0x70);
    addMessage(MIDI_CC | 0x01, 0x10, 0x75);
    addMessage(MIDI_CC | 0x01, 0x10, 0x62);
    addMessage(MIDI_CC | 0x01, 0x10, 0x52);
    addMessage(MIDI_CC | 0x01, 0x10, 0x48);
    addMessage(MIDI_CC | 0x01, 0x10, 0x46);
    addMessage(MIDI_CC | 0x01, 0x10, 0x41);

    ConfigKey control("[Test]", "SomeControl");
    MidiInputMappings mappings =
            LearningUtils::guessMidiInputMappings(control, m_messages);

    ASSERT_EQ(1, mappings.size());
    EXPECT_EQ(MidiInputMapping(MidiKey(MIDI_CC | 0x01, 0x10),
                      MidiOptions(),
                      control,
                      mappings.first().description),
            mappings.first());
}

TEST_F(LearningUtilsTest, CC7BitKnob_ConfusableForSpread64Ticker_0x40Included) {
    // Moving a CC knob through its range multiple times is confusable for
    // Spread64 select knobs when a 0x41 or 0x3F is repeated.
    addMessage(MIDI_CC | 0x01, 0x10, 0x70);
    addMessage(MIDI_CC | 0x01, 0x10, 0x60);
    addMessage(MIDI_CC | 0x01, 0x10, 0x50);
    addMessage(MIDI_CC | 0x01, 0x10, 0x41);
    addMessage(MIDI_CC | 0x01, 0x10, 0x3F);
    addMessage(MIDI_CC | 0x01, 0x10, 0x30);
    addMessage(MIDI_CC | 0x01, 0x10, 0x20);
    addMessage(MIDI_CC | 0x01, 0x10, 0x10);
    addMessage(MIDI_CC | 0x01, 0x10, 0x00);
    addMessage(MIDI_CC | 0x01, 0x10, 0x10);
    addMessage(MIDI_CC | 0x01, 0x10, 0x20);
    addMessage(MIDI_CC | 0x01, 0x10, 0x30);
    addMessage(MIDI_CC | 0x01, 0x10, 0x3F);
    addMessage(MIDI_CC | 0x01, 0x10, 0x41);
    // 0x40 tells us this is not a Spread64 select knob.
    addMessage(MIDI_CC | 0x01, 0x10, 0x40);
    addMessage(MIDI_CC | 0x01, 0x10, 0x50);
    addMessage(MIDI_CC | 0x01, 0x10, 0x60);
    addMessage(MIDI_CC | 0x01, 0x10, 0x70);

    ConfigKey control("[Test]", "SomeControl");
    MidiInputMappings mappings =
            LearningUtils::guessMidiInputMappings(control, m_messages);

    ASSERT_EQ(1, mappings.size());
    EXPECT_EQ(MidiInputMapping(MidiKey(MIDI_CC | 0x01, 0x10),
                      MidiOptions(),
                      control,
                      mappings.first().description),
            mappings.first());
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

    ConfigKey control("[Test]", "SomeControl");
    MidiInputMappings mappings =
            LearningUtils::guessMidiInputMappings(control, m_messages);

    ASSERT_EQ(1, mappings.size());
    MidiOptions options;
    options.selectknob = true;
    EXPECT_EQ(MidiInputMapping(MidiKey(MIDI_CC | 0x01, 0x10),
                      options,
                      control,
                      mappings.first().description),
            mappings.first());
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

    ConfigKey control("[Test]", "SomeControl");
    MidiInputMappings mappings =
            LearningUtils::guessMidiInputMappings(control, m_messages);

    ASSERT_EQ(1, mappings.size());
    MidiOptions options;
    options.spread64 = true;
    EXPECT_EQ(MidiInputMapping(MidiKey(MIDI_CC | 0x01, 0x10),
                      options,
                      control,
                      mappings.first().description),
            mappings.first());
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

    ConfigKey control("[Test]", "SomeControl");
    MidiInputMappings mappings =
            LearningUtils::guessMidiInputMappings(control, m_messages);
    ASSERT_EQ(1, mappings.size());
    EXPECT_EQ(MidiInputMapping(MidiKey(MIDI_CC | 0x01, 0x10),
                      options,
                      control,
                      mappings.first().description),
            mappings.first());

    m_messages.clear();

    // Status: 0x81 Control: 0x10
    addMessage(MIDI_CC | 0x01, 0x10, 0x7F);
    addMessage(MIDI_CC | 0x01, 0x10, 0x7E);
    addMessage(MIDI_CC | 0x01, 0x10, 0x7F);
    addMessage(MIDI_CC | 0x01, 0x10, 0x7C);

    mappings = LearningUtils::guessMidiInputMappings(control, m_messages);
    ASSERT_EQ(1, mappings.size());
    EXPECT_EQ(MidiInputMapping(MidiKey(MIDI_CC | 0x01, 0x10),
                      options,
                      control,
                      mappings.first().description),
            mappings.first());
}

TEST_F(LearningUtilsTest, SingleMessageSwitchMode_NoteOn) {
    // If we only get one NOTE_ON message that is either 0x7F or 0x00 then we
    // assume this is a binary toggle switch.

    // Status: 0x91 Control: 0x10
    addMessage(MIDI_NOTE_ON | 0x01, 0x10, 0x7F);

    ConfigKey control("[Test]", "SomeControl");
    MidiInputMappings mappings =
            LearningUtils::guessMidiInputMappings(control, m_messages);

    ASSERT_EQ(1, mappings.size());
    MidiOptions options;
    options.sw = true;
    EXPECT_EQ(MidiInputMapping(MidiKey(MIDI_NOTE_ON | 0x01, 0x10),
                      options,
                      control,
                      mappings.first().description),
            mappings.first());

    m_messages.clear();

    // Status: 0x91 Control: 0x10
    addMessage(MIDI_NOTE_ON | 0x01, 0x10, 0x00);

    mappings = LearningUtils::guessMidiInputMappings(control, m_messages);

    ASSERT_EQ(1, mappings.size());
    EXPECT_EQ(MidiInputMapping(MidiKey(MIDI_NOTE_ON | 0x01, 0x10),
                      options,
                      control,
                      mappings.first().description),
            mappings.first());
}

TEST_F(LearningUtilsTest, SingleMessageSwitchMode_CC) {
    // If we only get one CC message that is either 0x7F or 0x00 then we
    // assume this is a binary toggle switch.

    // Status: 0xB1 Control: 0x10
    addMessage(MIDI_CC | 0x01, 0x10, 0x7F);

    ConfigKey control("[Test]", "SomeControl");
    MidiInputMappings mappings =
            LearningUtils::guessMidiInputMappings(control, m_messages);

    ASSERT_EQ(1, mappings.size());
    MidiOptions options;
    options.sw = true;
    EXPECT_EQ(MidiInputMapping(MidiKey(MIDI_CC | 0x01, 0x10),
                      options,
                      control,
                      mappings.first().description),
            mappings.first());

    m_messages.clear();

    // Status: 0x91 Control: 0x10
    addMessage(MIDI_CC | 0x01, 0x10, 0x00);

    mappings = LearningUtils::guessMidiInputMappings(control, m_messages);

    ASSERT_EQ(1, mappings.size());
    EXPECT_EQ(MidiInputMapping(MidiKey(MIDI_CC | 0x01, 0x10),
                      options,
                      control,
                      mappings.first().description),
            mappings.first());
}

TEST_F(LearningUtilsTest, MultipleControlsUnrecognized_BindsFirst) {
    // Status 0x91, Control 0x10
    addMessage(MIDI_NOTE_ON | 0x01, 0x10, 0x7F);
    addMessage(MIDI_NOTE_ON | 0x01, 0x10, 0x00);

    // Status 0x91, Control 0x11
    addMessage(MIDI_NOTE_ON | 0x01, 0x11, 0x7F);
    addMessage(MIDI_NOTE_ON | 0x01, 0x11, 0x00);

    ConfigKey control("[Test]", "SomeControl");
    MidiInputMappings mappings =
            LearningUtils::guessMidiInputMappings(control, m_messages);
    ASSERT_EQ(1, mappings.size());
    EXPECT_EQ(MidiInputMapping(MidiKey(MIDI_NOTE_ON | 0x01, 0x10),
                      MidiOptions(),
                      control,
                      mappings.first().description),
            mappings.first());
}

TEST_F(LearningUtilsTest, MultipleChannelsUnrecognized_BindsFirst) {
    // Status 0x91, Control 0x10
    addMessage(MIDI_NOTE_ON | 0x01, 0x10, 0x7F);
    addMessage(MIDI_NOTE_ON | 0x01, 0x10, 0x00);

    // Status 0x91, Control 0x11
    addMessage(MIDI_NOTE_ON | 0x02, 0x10, 0x7F);
    addMessage(MIDI_NOTE_ON | 0x02, 0x10, 0x00);

    ConfigKey control("[Test]", "SomeControl");
    MidiInputMappings mappings =
            LearningUtils::guessMidiInputMappings(control, m_messages);
    ASSERT_EQ(1, mappings.size());
    EXPECT_EQ(MidiInputMapping(MidiKey(MIDI_NOTE_ON | 0x01, 0x10),
                      MidiOptions(),
                      control,
                      mappings.first().description),
            mappings.first());
}
