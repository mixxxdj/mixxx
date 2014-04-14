#include <QScopedPointer>

#include <gmock/gmock.h>

#include "test/mixxxtest.h"
#include "controllers/midi/midicontroller.h"
#include "controllers/midi/midicontrollerpreset.h"
#include "controllers/midi/midimessage.h"
#include "controlpushbutton.h"
#include "controlpotmeter.h"

class MockMidiController : public MidiController {
  public:
    MockMidiController() { }
    virtual ~MockMidiController() { }

    MOCK_METHOD0(open, int());
    MOCK_METHOD0(close, int());
    MOCK_METHOD1(sendWord, void(unsigned int work));
    MOCK_METHOD1(send, void(QByteArray data));
    MOCK_CONST_METHOD0(isPolling, bool());
};

class MidiControllerTest : public MixxxTest {
  protected:
    virtual void SetUp() {
        m_pController.reset(new MockMidiController());
    }

    virtual void TearDown() {
    }

    void addMapping(MidiInputMapping mapping) {
        m_preset.inputMappings.insertMulti(mapping.key.key, mapping);
    }

    void loadPreset(const MidiControllerPreset& preset) {
        m_pController->visit(&preset);
    }

    void receive(unsigned char status, unsigned char control,
                 unsigned char value) {
        m_pController->receive(status, control, value);
    }

    MidiControllerPreset m_preset;
    QScopedPointer<MockMidiController> m_pController;
};

TEST_F(MidiControllerTest, ReceiveMessage_PushButton_OnOff) {
    // Most MIDI controller send push-buttons as (NOTE_ON, 0x7F) for press and
    // (NOTE_OFF, 0x00) for release.
    ConfigKey key("[Channel1]", "hotcue_1_activate");
    ControlPushButton cpb(key);

    unsigned char channel = 0x01;
    unsigned char control = 0x10;

    addMapping(MidiInputMapping(MidiKey(MIDI_NOTE_ON | channel, control),
                                MidiOptions(), key));
    addMapping(MidiInputMapping(MidiKey(MIDI_NOTE_OFF | channel, control),
                                MidiOptions(), key));
    loadPreset(m_preset);

    // Receive an on/off, sets the control on/off with each press.
    receive(MIDI_NOTE_ON | channel, control, 0x7F);
    EXPECT_LT(0.0, cpb.get());
    receive(MIDI_NOTE_OFF | channel, control, 0x00);
    EXPECT_DOUBLE_EQ(0.0, cpb.get());

    // Receive an on/off, sets the control on/off with each press.
    receive(MIDI_NOTE_ON | channel, control, 0x7F);
    EXPECT_LT(0.0, cpb.get());
    receive(MIDI_NOTE_OFF | channel, control, 0x00);
    EXPECT_DOUBLE_EQ(0.0, cpb.get());
}

TEST_F(MidiControllerTest, ReceiveMessage_PushButton_OnOn) {
    // Some MIDI controllers send push-buttons as (NOTE_ON, 0x7f) for press and
    // (NOTE_ON, 0x00) for release.
    ConfigKey key("[Channel1]", "hotcue_1_activate");
    ControlPushButton cpb(key);

    unsigned char channel = 0x01;
    unsigned char control = 0x10;

    addMapping(MidiInputMapping(MidiKey(MIDI_NOTE_ON | channel, control),
                                MidiOptions(), key));
    loadPreset(m_preset);

    // Receive an on/off, sets the control on/off with each press.
    receive(MIDI_NOTE_ON | channel, control, 0x7F);
    EXPECT_LT(0.0, cpb.get());
    receive(MIDI_NOTE_ON | channel, control, 0x00);
    EXPECT_DOUBLE_EQ(0.0, cpb.get());

    // Receive an on/off, sets the control on/off with each press.
    receive(MIDI_NOTE_ON | channel, control, 0x7F);
    EXPECT_LT(0.0, cpb.get());
    receive(MIDI_NOTE_ON | channel, control, 0x00);
    EXPECT_DOUBLE_EQ(0.0, cpb.get());
}

TEST_F(MidiControllerTest, ReceiveMessage_PushButton_CC) {
    // Some MIDI controllers (e.g. Korg nanoKONTROL) send momentary push-buttons
    // as (CC, 0x7f) for press and (CC, 0x00) for release.
    ConfigKey key("[Channel1]", "hotcue_1_activate");
    ControlPushButton cpb(key);

    unsigned char channel = 0x01;
    unsigned char control = 0x10;

    addMapping(MidiInputMapping(MidiKey(MIDI_CC | channel, control),
                                MidiOptions(), key));
    loadPreset(m_preset);

    // Receive an on/off, sets the control on/off with each press.
    receive(MIDI_CC | channel, control, 0x7F);
    EXPECT_LT(0.0, cpb.get());
    receive(MIDI_CC | channel, control, 0x00);
    EXPECT_DOUBLE_EQ(0.0, cpb.get());

    // Receive an on/off, sets the control on/off with each press.
    receive(MIDI_CC | channel, control, 0x7F);
    EXPECT_LT(0.0, cpb.get());
    receive(MIDI_CC | channel, control, 0x00);
    EXPECT_DOUBLE_EQ(0.0, cpb.get());
}

TEST_F(MidiControllerTest, ReceiveMessage_ToggleButton_OnOff) {
    // Most MIDI controller send push-buttons as (NOTE_ON, 0x7F) for press and
    // (NOTE_OFF, 0x00) for release.
    ConfigKey key("[Channel1]", "keylock");
    ControlPushButton cpb(key);
    cpb.setButtonMode(ControlPushButton::TOGGLE);

    unsigned char channel = 0x01;
    unsigned char control = 0x10;

    addMapping(MidiInputMapping(MidiKey(MIDI_NOTE_ON | channel, control),
                                MidiOptions(), key));
    addMapping(MidiInputMapping(MidiKey(MIDI_NOTE_OFF | channel, control),
                                MidiOptions(), key));
    loadPreset(m_preset);

    // Receive an on/off, toggles the control.
    receive(MIDI_NOTE_ON | channel, control, 0x7F);
    receive(MIDI_NOTE_OFF | channel, control, 0x00);

    EXPECT_LT(0.0, cpb.get());

    // Receive an on/off, toggles the control.
    receive(MIDI_NOTE_ON | channel, control, 0x7F);
    receive(MIDI_NOTE_OFF | channel, control, 0x00);

    EXPECT_DOUBLE_EQ(0.0, cpb.get());
}

TEST_F(MidiControllerTest, ReceiveMessage_ToggleButton_OnOn) {
    // Some MIDI controllers send push-buttons as (NOTE_ON, 0x7f) for press and
    // (NOTE_ON, 0x00) for release.
    ConfigKey key("[Channel1]", "keylock");
    ControlPushButton cpb(key);
    cpb.setButtonMode(ControlPushButton::TOGGLE);

    unsigned char channel = 0x01;
    unsigned char control = 0x10;

    addMapping(MidiInputMapping(MidiKey(MIDI_NOTE_ON | channel, control),
                                MidiOptions(), key));
    loadPreset(m_preset);

    // Receive an on/off, toggles the control.
    receive(MIDI_NOTE_ON | channel, control, 0x7F);
    receive(MIDI_NOTE_ON | channel, control, 0x00);

    EXPECT_LT(0.0, cpb.get());

    // Receive an on/off, toggles the control.
    receive(MIDI_NOTE_ON | channel, control, 0x7F);
    receive(MIDI_NOTE_ON | channel, control, 0x00);

    EXPECT_DOUBLE_EQ(0.0, cpb.get());
}

TEST_F(MidiControllerTest, ReceiveMessage_ToggleButton_CC) {
    // Some MIDI controllers (e.g. Korg nanoKONTROL) send momentary push-buttons
    // as (CC, 0x7f) for press and (CC, 0x00) for release.
    ConfigKey key("[Channel1]", "keylock");
    ControlPushButton cpb(key);
    cpb.setButtonMode(ControlPushButton::TOGGLE);

    unsigned char channel = 0x01;
    unsigned char control = 0x10;

    addMapping(MidiInputMapping(MidiKey(MIDI_CC | channel, control),
                                MidiOptions(), key));
    loadPreset(m_preset);

    // Receive an on/off, toggles the control.
    receive(MIDI_CC | channel, control, 0x7F);
    receive(MIDI_CC | channel, control, 0x00);

    EXPECT_LT(0.0, cpb.get());

    // Receive an on/off, toggles the control.
    receive(MIDI_CC | channel, control, 0x7F);
    receive(MIDI_CC | channel, control, 0x00);

    EXPECT_DOUBLE_EQ(0.0, cpb.get());
}

TEST_F(MidiControllerTest, ReceiveMessage_PotMeter) {
    ConfigKey key("[Channel1]", "playposition");

    const double kMinValue = -1234.5;
    const double kMaxValue = 678.9;
    ControlPotmeter cpb(key, kMinValue, kMaxValue);

    unsigned char channel = 0x01;
    unsigned char control = 0x10;

    addMapping(MidiInputMapping(MidiKey(MIDI_CC | channel, control),
                                MidiOptions(), key));
    loadPreset(m_preset);

    // Receive a 0, MIDI parameter should map to the min value.
    receive(MIDI_CC | channel, control, 0x00);
    EXPECT_DOUBLE_EQ(kMinValue, cpb.get());

    // Receive a 0x7F, MIDI parameter should map to the potmeter max value.
    receive(MIDI_CC | channel, control, 0x7F);
    EXPECT_DOUBLE_EQ(kMaxValue, cpb.get());

    // Receive a 0x40, MIDI parameter should map to the potmeter middle value.
    receive(MIDI_CC | channel, control, 0x40);
    EXPECT_DOUBLE_EQ((kMinValue + kMaxValue) * 0.5, cpb.get());
}
