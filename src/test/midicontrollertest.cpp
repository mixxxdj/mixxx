#include <QScopedPointer>

#include <gmock/gmock.h>

#include "test/mixxxtest.h"
#include "controllers/midi/midicontroller.h"
#include "controllers/midi/midicontrollerpreset.h"
#include "controllers/midi/midimessage.h"
#include "control/controlpushbutton.h"
#include "control/controlpotmeter.h"
#include "util/time.h"

class MockMidiController : public MidiController {
  public:
    explicit MockMidiController(UserSettingsPointer pConfig)
            : MidiController(pConfig) {
    }
    ~MockMidiController() override { }

    MOCK_METHOD0(open, int());
    MOCK_METHOD0(close, int());
    MOCK_METHOD3(sendShortMsg, void(unsigned char status,
                                    unsigned char byte1,
                                    unsigned char byte2));
    MOCK_METHOD1(send, void(const QByteArray& data));
    MOCK_CONST_METHOD0(isPolling, bool());
};

class MidiControllerTest : public MixxxTest {
  protected:
    void SetUp() override {
        m_pController.reset(new MockMidiController(config()));
    }

    void addMapping(const MidiInputMapping& mapping) {
        m_preset.addInputMapping(mapping.key.key, mapping);
    }

    void loadPreset(const MidiControllerPreset& preset) {
        m_pController->visit(&preset);
    }

    void receive(unsigned char status, unsigned char control,
                 unsigned char value) {
        // TODO(rryan): This test doesn't care about timestamps.
        m_pController->receive(status, control, value, mixxx::Time::elapsed());
    }

    MidiControllerPreset m_preset;
    QScopedPointer<MockMidiController> m_pController;
};

TEST_F(MidiControllerTest, ReceiveMessage_PushButtonCO_PushOnOff) {
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

TEST_F(MidiControllerTest, ReceiveMessage_PushButtonCO_PushOnOn) {
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

TEST_F(MidiControllerTest, ReceiveMessage_PushButtonCO_ToggleOnOff_ButtonMidiOption) {
    // Using the button MIDI option allows you to use a MIDI toggle button as a
    // push button.
    ConfigKey key("[Channel1]", "hotcue_1_activate");
    ControlPushButton cpb(key);

    unsigned char channel = 0x01;
    unsigned char control = 0x10;

    MidiOptions options;
    options.button = true;

    addMapping(MidiInputMapping(MidiKey(MIDI_NOTE_ON | channel, control),
                                options, key));
    addMapping(MidiInputMapping(MidiKey(MIDI_NOTE_OFF | channel, control),
                                options, key));
    loadPreset(m_preset);

    // NOTE(rryan): This behavior is broken!

    // Toggle the switch on, sets the push button on.
    receive(MIDI_NOTE_ON | channel, control, 0x7F);
    EXPECT_LT(0.0, cpb.get());

    // The push button is stuck down here!

    // Toggle the switch off, sets the push button off.
    receive(MIDI_NOTE_OFF | channel, control, 0x00);
    EXPECT_DOUBLE_EQ(0.0, cpb.get());
}

TEST_F(MidiControllerTest, ReceiveMessage_PushButtonCO_ToggleOnOff_SwitchMidiOption) {
    // Using the switch MIDI option interprets a MIDI toggle button as a toggle
    // button rather than a momentary push button.
    ConfigKey key("[Channel1]", "hotcue_1_activate");
    ControlPushButton cpb(key);

    unsigned char channel = 0x01;
    unsigned char control = 0x10;

    MidiOptions options;
    options.sw = true;

    addMapping(MidiInputMapping(MidiKey(MIDI_NOTE_ON | channel, control),
                                options, key));
    addMapping(MidiInputMapping(MidiKey(MIDI_NOTE_OFF | channel, control),
                                options, key));
    loadPreset(m_preset);

    // NOTE(rryan): This behavior is broken!

    // Toggle the switch on, sets the push button on.
    receive(MIDI_NOTE_ON | channel, control, 0x7F);
    EXPECT_LT(0.0, cpb.get());

    // The push button is stuck down here!

    // Toggle the switch off, sets the push button on again.
    receive(MIDI_NOTE_OFF | channel, control, 0x00);
    EXPECT_LT(0.0, cpb.get());

    // NOTE(rryan): What is supposed to happen in this case? It's an open
    // question I think. I think if you want to connect a switch MIDI control to
    // a push button CO then the switch should directly set the CO. After all,
    // the preset author asked for the switch to be interpreted as a switch. If
    // they want the switch to act like a push button, they should use the
    // button MIDI option.
    //
    // Most of our push buttons trigger behavior on press and do nothing on
    // release, and most don't care about being "stuck down" except for hotcue
    // and cue controls that have preview behavior.

    // "reverse" is an example of a push button that is a push button because we
    // want the default behavior to be momentary press and not toggle. If I
    // mapped a switch to it, I would expect the switch to enable it (set it to
    // 1) when the switch was enabled and set it to 0 when the switch was
    // disabled. So I think we should change the switch option to behave like
    // this.
}

TEST_F(MidiControllerTest, ReceiveMessage_PushButtonCO_PushCC) {
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

TEST_F(MidiControllerTest, ReceiveMessage_ToggleCO_PushOnOff) {
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

TEST_F(MidiControllerTest, ReceiveMessage_ToggleCO_PushOnOn) {
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

TEST_F(MidiControllerTest, ReceiveMessage_ToggleCO_ToggleOnOff_ButtonMidiOption) {
    // Using the button MIDI option allows you to use a MIDI toggle button as a
    // push button.
    ConfigKey key("[Channel1]", "keylock");
    ControlPushButton cpb(key);
    cpb.setButtonMode(ControlPushButton::TOGGLE);

    unsigned char channel = 0x01;
    unsigned char control = 0x10;

    MidiOptions options;
    options.button = true;

    addMapping(MidiInputMapping(MidiKey(MIDI_NOTE_ON | channel, control),
                                options, key));
    addMapping(MidiInputMapping(MidiKey(MIDI_NOTE_OFF | channel, control),
                                options, key));
    loadPreset(m_preset);

    // NOTE(rryan): If the intended behavior of the button MIDI option is to
    // make a toggle MIDI button act like a push button then this isn't
    // working. The toggle on toggles the CO but the toggle off does nothing.

    // Toggle the switch on, since it is interpreted as a button press it
    // toggles the button on.
    receive(MIDI_NOTE_ON | channel, control, 0x7F);
    EXPECT_LT(0.0, cpb.get());

    // Toggle the switch off, since it is interpreted as a button release it
    // does nothing to the toggle button.
    receive(MIDI_NOTE_OFF | channel, control, 0x00);
    EXPECT_LT(0.0, cpb.get());
}

TEST_F(MidiControllerTest, ReceiveMessage_ToggleCO_ToggleOnOff_SwitchMidiOption) {
    // Using the switch MIDI option interprets a MIDI toggle button as a toggle
    // button rather than a momentary push button.
    ConfigKey key("[Channel1]", "keylock");
    ControlPushButton cpb(key);
    cpb.setButtonMode(ControlPushButton::TOGGLE);

    unsigned char channel = 0x01;
    unsigned char control = 0x10;

    MidiOptions options;
    options.sw = true;

    addMapping(MidiInputMapping(MidiKey(MIDI_NOTE_ON | channel, control),
                                options, key));
    addMapping(MidiInputMapping(MidiKey(MIDI_NOTE_OFF | channel, control),
                                options, key));
    loadPreset(m_preset);

    // NOTE(rryan): If the intended behavior of switch MIDI option is to make a
    // toggle MIDI button act like a toggle button then this isn't working. The
    // toggle on presses the CO and the toggle off presses the CO. This toggles
    // the control but allows it to get out of sync.

    // Toggle the switch on, since it is interpreted as a button press it
    // toggles the control on.
    receive(MIDI_NOTE_ON | channel, control, 0x7F);
    EXPECT_LT(0.0, cpb.get());

    // Toggle the switch off, since it is interpreted as a button press it
    // toggles the control off.
    receive(MIDI_NOTE_OFF | channel, control, 0x00);
    EXPECT_DOUBLE_EQ(0.0, cpb.get());

    // Meanwhile, the GUI toggles the control on again.
    // NOTE(rryan): Now the MIDI toggle button is out of sync with the toggle
    // CO.
    cpb.set(1.0);

    // Toggle the switch on, since it is interpreted as a button press it
    // toggles the control off (since it was on).
    receive(MIDI_NOTE_ON | channel, control, 0x7F);
    EXPECT_DOUBLE_EQ(0.0, cpb.get());

    // Toggle the switch off, since it is interpreted as a button press it
    // toggles the control on (since it was off).
    receive(MIDI_NOTE_OFF | channel, control, 0x00);
    EXPECT_LT(0.0, cpb.get());
}

TEST_F(MidiControllerTest, ReceiveMessage_ToggleCO_PushCC) {
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

TEST_F(MidiControllerTest, ReceiveMessage_PotMeterCO_7BitCC) {
    ConfigKey key("[Channel1]", "playposition");

    const double kMinValue = -1234.5;
    const double kMaxValue = 678.9;
    const double kMiddleValue = (kMinValue + kMaxValue) * 0.5;
    ControlPotmeter potmeter(key, kMinValue, kMaxValue);

    unsigned char channel = 0x01;
    unsigned char control = 0x10;

    addMapping(MidiInputMapping(MidiKey(MIDI_CC | channel, control),
                                MidiOptions(), key));
    loadPreset(m_preset);

    // Receive a 0, MIDI parameter should map to the min value.
    receive(MIDI_CC | channel, control, 0x00);
    EXPECT_DOUBLE_EQ(kMinValue, potmeter.get());

    // Receive a 0x7F, MIDI parameter should map to the potmeter max value.
    receive(MIDI_CC | channel, control, 0x7F);
    EXPECT_DOUBLE_EQ(kMaxValue, potmeter.get());

    // Receive a 0x40, MIDI parameter should map to the potmeter middle value.
    receive(MIDI_CC | channel, control, 0x40);
    EXPECT_DOUBLE_EQ(kMiddleValue, potmeter.get());
}

TEST_F(MidiControllerTest, ReceiveMessage_PotMeterCO_14BitCC) {
    ConfigKey key("[Channel1]", "playposition");

    const double kMinValue = -1234.5;
    const double kMaxValue = 678.9;
    const double kMiddleValue = (kMinValue + kMaxValue) * 0.5;
    ControlPotmeter potmeter(key, kMinValue, kMaxValue);
    potmeter.set(0);

    unsigned char channel = 0x01;
    unsigned char lsb_control = 0x10;
    unsigned char msb_control = 0x11;

    MidiOptions lsb;
    lsb.fourteen_bit_lsb = true;

    MidiOptions msb;
    msb.fourteen_bit_msb = true;

    addMapping(MidiInputMapping(MidiKey(MIDI_CC | channel, lsb_control),
                                lsb, key));
    addMapping(MidiInputMapping(MidiKey(MIDI_CC | channel, msb_control),
                                msb, key));
    loadPreset(m_preset);

    // If kMinValue or kMaxValue are such that the middle value is 0 then the
    // set(0) commands below allow us to hide failures.
    ASSERT_NE(0.0, kMiddleValue);

    // Receive a 0x0000 (lsb-first), MIDI parameter should map to the min value.
    potmeter.set(0);
    receive(MIDI_CC | channel, lsb_control, 0x00);
    receive(MIDI_CC | channel, msb_control, 0x00);
    EXPECT_DOUBLE_EQ(kMinValue, potmeter.get());

    // Receive a 0x0000 (msb-first), MIDI parameter should map to the min value.
    potmeter.set(0);
    receive(MIDI_CC | channel, msb_control, 0x00);
    receive(MIDI_CC | channel, lsb_control, 0x00);
    EXPECT_DOUBLE_EQ(kMinValue, potmeter.get());

    // Receive a 0x3FFF (lsb-first), MIDI parameter should map to the max value.
    potmeter.set(0);
    receive(MIDI_CC | channel, lsb_control, 0x7F);
    receive(MIDI_CC | channel, msb_control, 0x7F);
    EXPECT_DOUBLE_EQ(kMaxValue, potmeter.get());

    // Receive a 0x3FFF (msb-first), MIDI parameter should map to the max value.
    potmeter.set(0);
    receive(MIDI_CC | channel, msb_control, 0x7F);
    receive(MIDI_CC | channel, lsb_control, 0x7F);
    EXPECT_DOUBLE_EQ(kMaxValue, potmeter.get());

    // Receive a 0x2000 (lsb-first), MIDI parameter should map to the middle
    // value.
    potmeter.set(0);
    receive(MIDI_CC | channel, lsb_control, 0x00);
    receive(MIDI_CC | channel, msb_control, 0x40);
    EXPECT_DOUBLE_EQ(kMiddleValue, potmeter.get());

    // Receive a 0x2000 (msb-first), MIDI parameter should map to the middle
    // value.
    potmeter.set(0);
    receive(MIDI_CC | channel, msb_control, 0x40);
    receive(MIDI_CC | channel, lsb_control, 0x00);
    EXPECT_DOUBLE_EQ(kMiddleValue, potmeter.get());

    // Check the 14-bit resolution is actually present. Receive a 0x2001
    // (msb-first), MIDI parameter should map to the middle value plus a tiny
    // amount. Scaling is not quite linear for MIDI parameters so just check
    // that incrementing the LSB by 1 is greater than the middle value.
    potmeter.set(0);
    receive(MIDI_CC | channel, msb_control, 0x40);
    receive(MIDI_CC | channel, lsb_control, 0x01);
    EXPECT_LT(kMiddleValue, potmeter.get());

    // Check the 14-bit resolution is actually present. Receive a 0x2001
    // (lsb-first), MIDI parameter should map to the middle value plus a tiny
    // amount. Scaling is not quite linear for MIDI parameters so just check
    // that incrementing the LSB by 1 is greater than the middle value.
    potmeter.set(0);
    receive(MIDI_CC | channel, lsb_control, 0x01);
    receive(MIDI_CC | channel, msb_control, 0x40);
    EXPECT_LT(kMiddleValue, potmeter.get());
}

TEST_F(MidiControllerTest, ReceiveMessage_PotMeterCO_14BitPitchBend) {
    ConfigKey key("[Channel1]", "rate");

    const double kMinValue = -1234.5;
    const double kMaxValue = 678.9;
    const double kMiddleValue = (kMinValue + kMaxValue) * 0.5;
    ControlPotmeter potmeter(key, kMinValue, kMaxValue);
    unsigned char channel = 0x01;

    // The control is ignored in mappings for messages where the control is part
    // of the payload.
    addMapping(MidiInputMapping(MidiKey(MIDI_PITCH_BEND | channel, 0xFF),
                                MidiOptions(), key));
    loadPreset(m_preset);

    // Receive a 0x0000, MIDI parameter should map to the min value.
    receive(MIDI_PITCH_BEND | channel, 0x00, 0x00);
    EXPECT_DOUBLE_EQ(kMinValue, potmeter.get());

    // Receive a 0x3FFF, MIDI parameter should map to the potmeter max value.
    receive(MIDI_PITCH_BEND | channel, 0x7F, 0x7F);
    EXPECT_DOUBLE_EQ(kMaxValue, potmeter.get());

    // Receive a 0x2000, MIDI parameter should map to the potmeter middle value.
    receive(MIDI_PITCH_BEND | channel, 0x00, 0x40);
    EXPECT_DOUBLE_EQ(kMiddleValue, potmeter.get());

    // Check the 14-bit resolution is actually present. Receive a 0x2001, MIDI
    // parameter should map to the middle value plus a tiny amount. Scaling is
    // not quite linear for MIDI parameters so just check that incrementing the
    // LSB by 1 is greater than the middle value.
    receive(MIDI_PITCH_BEND | channel, 0x01, 0x40);
    EXPECT_LT(kMiddleValue, potmeter.get());
}
