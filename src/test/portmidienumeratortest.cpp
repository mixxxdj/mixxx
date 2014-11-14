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

    // Bug 1033375 - contain string 'to' in device name, strip
    // ' input port ' and ' output port ' from strings
    ASSERT_TRUE(shouldLinkInputToOutput(
        "Traktor Kontrol X1 - 1 MIDI input port 0",
        "Traktor Kontrol X1 - 1 MIDI output port 0"));

    // Bug 1033375 - contain string 'to' in device name, strip
    // ' input port ' and ' output port ' from strings not done
    // because of invalid strings
    ASSERT_TRUE(shouldLinkInputToOutput(
        "Traktor Kontrol X1 - 1 MIDI input 0",
        "Traktor Kontrol X1 - 1 MIDI output 0"));

    ASSERT_TRUE(shouldLinkInputToOutput(
        "Pure Data Midi in 1",
        "Pure Data Midi out 1"));

    // Strip ' input ' from inputs and ' output ' from outputs.

    // Lemur Daemon shows 8 port pairs named like: 'Daemon Input 1' and 'Daemon
    // Output 2'
    ASSERT_TRUE(shouldLinkInputToOutput("Daemon Input 1", "Daemon Output 1"));
    ASSERT_TRUE(shouldLinkInputToOutput("Daemon Input 2", "Daemon Output 2"));
    ASSERT_TRUE(shouldLinkInputToOutput("Daemon InPuT 2", "Daemon OuTpuT 2"));
    ASSERT_TRUE(shouldLinkInputToOutput("Daemon Input 1234", "Daemon Output 1234"));
    ASSERT_TRUE(shouldLinkInputToOutput("Daemon Input ", "Daemon Output "));
    ASSERT_TRUE(shouldLinkInputToOutput("Daemon Input 1", "daemon output 1"));
    ASSERT_TRUE(shouldLinkInputToOutput("Daemon Input 1", "Daemon Input 1"));
    ASSERT_FALSE(shouldLinkInputToOutput("Daemon Input", "Daemon Output"));
    ASSERT_FALSE(shouldLinkInputToOutput("Daemon Input 1234", "Daemon Output 5678"));
    ASSERT_FALSE(shouldLinkInputToOutput("Daemon Input 1", "Daemon Output 1 "));
    ASSERT_FALSE(shouldLinkInputToOutput("Daemon Input 1", "Daemon Output 11"));
    ASSERT_FALSE(shouldLinkInputToOutput("Daemon Input 2", "Daemon Output 12"));

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
