#include <gtest/gtest.h>
#include <QtDebug>

#include "controllers/midi/portmidienumerator.h"

class PortMidiEnumeratorTest : public testing::Test {
  protected:
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }
};

TEST_F(PortMidiEnumeratorTest, InputOutputPortsLinked) {
    // Identical device names should link.
    ASSERT_TRUE(shouldLinkInputToOutput(
        "Vestax VCI-100",
        "Vestax VCI-100"));

    // Different device names should not link.
    ASSERT_FALSE(shouldLinkInputToOutput(
        "Vestax VCI-100",
        "Vestax VCI-300"));

    // Ports with From or To in them that are otherwise equal should link.
    ASSERT_TRUE(shouldLinkInputToOutput(
        "From: Vestax VCI-100",
        "To: Vestax VCI-100"));

    // Ports with From or To in them that are otherwise equal should link. Case
    // does not matter.
    ASSERT_TRUE(shouldLinkInputToOutput(
        "frOm: Vestax VCI-100",
        "tO: Vestax VCI-100"));

    // Ports that match the pattern "$DEVICE MIDI $INSTANCE ..." should link.

    ASSERT_TRUE(shouldLinkInputToOutput(
        "Vestax VCI-100 MIDI 1",
        "Vestax VCI-100 MIDI 1"));

    // Stuff after $INSTANCE doesn't matter.
    ASSERT_TRUE(shouldLinkInputToOutput(
        "Vestax VCI-100 MIDI 1 OtherJunk",
        "Vestax VCI-100 MIDI 1 123 Doesn't Matter"));

    // Name is case-insensitive.
    ASSERT_TRUE(shouldLinkInputToOutput(
        "Vestax VCI-100 MIDI 1 OtherJunk",
        "VesTaX VCI-100 MIDI 1 123 Doesn't Matter"));

    // Different $INSTANCE matters.
    ASSERT_FALSE(shouldLinkInputToOutput(
        "Vestax VCI-100 MIDI 1 OtherJunk",
        "Vestax VCI-100 MIDI 2 123 Doesn't Matter"));

    // Different $INSTANCE with no trailing stuff matters.
    ASSERT_FALSE(shouldLinkInputToOutput(
        "Vestax VCI-100 MIDI 1",
        "Vestax VCI-100 MIDI 12"));

    // Extra stuff only on one doesn't matter.
    ASSERT_TRUE(shouldLinkInputToOutput(
        "Vestax VCI-100 MIDI 12",
        "Vestax VCI-100 MIDI 12 123 Something Else"));

    // Dangling numerals after the device name but before MIDI are fine.
    ASSERT_TRUE(shouldLinkInputToOutput(
        "Vestax VCI-100 2 MIDI 12",
        "Vestax VCI-100 2 MIDI 12 123 Something Else"));

    // Different $DEVICE matters.
    ASSERT_FALSE(shouldLinkInputToOutput(
        "Vestax VCI-100 MIDI 1",
        "Vestax VCI-300 MIDI 1"));

    // Non-numeral extra stuff following a lone numeral after the device name
    // doesn't matter.
    ASSERT_TRUE(shouldLinkInputToOutput(
        "nanoKONTROL2 1 CTRL",
        "nanoKONTROL2 1 SLIDER/KNOB"));

    // Extra stuff with a numeral following a lone numeral after the device name
    // doesn't get linked.
    ASSERT_FALSE(shouldLinkInputToOutput(
        "nanoKONTROL2 1 1 CTRL",
        "nanoKONTROL2 1 SLIDER/KNOB"));

    // If the device name has a numeral dangling off of it we should not get
    // confused by that.
    ASSERT_FALSE(shouldLinkInputToOutput(
        "nanoKONTROL 2 1 CTRL",
        "nanoKONTROL 2 3 SLIDER/KNOB"));
}
