#include <gmock/gmock.h>

#include <QScopedPointer>

#include "control/controlpotmeter.h"
#include "control/controlpushbutton.h"
#include "controllers/midi/legacymidicontrollermapping.h"
#include "controllers/midi/midicontroller.h"
#include "controllers/midi/midimessage.h"
#include "controllers/midi/midiutils.h"
#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"
#include "test/mixxxtest.h"
#include "util/time.h"

class MockMidiController : public MidiController {
  public:
    explicit MockMidiController()
            : MidiController("test") {
    }
    ~MockMidiController() override {
    }

    MOCK_METHOD1(open, int(const QString& resourcePath));
    MOCK_METHOD0(close, int());
    MOCK_METHOD3(sendShortMsg,
            void(unsigned char status,
                    unsigned char byte1,
                    unsigned char byte2));
    MOCK_METHOD1(sendBytes, bool(const QByteArray& data));
    MOCK_CONST_METHOD0(isPolling, bool());

    PhysicalTransportProtocol getPhysicalTransportProtocol() const override {
        return PhysicalTransportProtocol::UNKNOWN;
    }
    DataRepresentationProtocol getDataRepresentationProtocol() const override {
        return DataRepresentationProtocol::MIDI;
    }

    QString getVendorString() const override {
        static const QString manufacturer = "Test Manufacturer";
        return manufacturer;
    }
    std::optional<uint16_t> getVendorId() const override {
        return std::nullopt;
    }

    QString getProductString() const override {
        static const QString product = "Test Product";
        return product;
    }
    std::optional<uint16_t> getProductId() const override {
        return std::nullopt;
    }

    QString getSerialNumber() const override {
        static const QString serialNumber = "123456789";
        return serialNumber;
    }

    std::optional<uint8_t> getUsbInterfaceNumber() const override {
        return std::nullopt;
    }
};

class MidiControllerTest : public MixxxTest {
  protected:
    void SetUp() override {
        m_pController.reset(new MockMidiController());
        m_pMapping = std::make_shared<LegacyMidiControllerMapping>();
        m_pController->startEngine();
        m_pController->m_pScriptEngineLegacy->initialize();
    }

    void addMapping(const MidiInputMapping& mapping) {
        m_pMapping->addInputMapping(mapping.key.key, mapping);
    }

    void receivedShortMessage(unsigned char status, unsigned char control, unsigned char value) {
        // TODO(rryan): This test doesn't care about timestamps.
        m_pController->receivedShortMessage(status, control, value, mixxx::Time::elapsed());
    }

    void receivedShortMessage(MidiOpCode opcode, uint8_t channel, uint8_t control, uint8_t value) {
        ASSERT_TRUE((channel & 0xF) == channel);
        receivedShortMessage(
                MidiUtils::statusFromOpCodeAndChannel(opcode, channel),
                control,
                value);
    }

    bool evaluateAndAssert(const QString& code) {
        return m_pController->m_pScriptEngineLegacy->jsEngine()->evaluate(code).isError();
    }

    int getInputMappingCount() {
        return m_pController->m_pMapping->getInputMappings().count();
    }

    void shutdownController() {
        m_pController->m_pScriptEngineLegacy->shutdown();
    }

    std::shared_ptr<LegacyMidiControllerMapping> m_pMapping;
    QScopedPointer<MockMidiController> m_pController;
};

TEST_F(MidiControllerTest, ReceiveMessage_PushButtonCO_PushOnOff) {
    // Most MIDI controller send push-buttons as (NOTE_ON, 0x7F) for press and
    // (NOTE_OFF, 0x00) for release.
    ConfigKey key("[Channel1]", "hotcue_1_activate");
    ControlPushButton cpb(key);

    unsigned char channel = 0x01;
    unsigned char control = 0x10;

    addMapping(MidiInputMapping(MidiKey(MidiUtils::statusFromOpCodeAndChannel(
                                                MidiOpCode::NoteOn, channel),
                                        control),
            MidiOptions(),
            key));
    addMapping(MidiInputMapping(MidiKey(MidiUtils::statusFromOpCodeAndChannel(
                                                MidiOpCode::NoteOff, channel),
                                        control),
            MidiOptions(),
            key));
    m_pController->setMapping(m_pMapping);

    // Receive an on/off, sets the control on/off with each press.
    receivedShortMessage(MidiOpCode::NoteOn, channel, control, 0x7F);
    EXPECT_LT(0.0, cpb.get());
    receivedShortMessage(MidiOpCode::NoteOff, channel, control, 0x00);
    EXPECT_DOUBLE_EQ(0.0, cpb.get());

    // Receive an on/off, sets the control on/off with each press.
    receivedShortMessage(MidiOpCode::NoteOn, channel, control, 0x7F);
    EXPECT_LT(0.0, cpb.get());
    receivedShortMessage(MidiOpCode::NoteOff, channel, control, 0x00);
    EXPECT_DOUBLE_EQ(0.0, cpb.get());
}

TEST_F(MidiControllerTest, ReceiveMessage_PushButtonCO_PushOnOn) {
    // Some MIDI controllers send push-buttons as (NOTE_ON, 0x7f) for press and
    // (NOTE_ON, 0x00) for release.
    ConfigKey key("[Channel1]", "hotcue_1_activate");
    ControlPushButton cpb(key);

    unsigned char channel = 0x01;
    unsigned char control = 0x10;

    addMapping(MidiInputMapping(MidiKey(MidiUtils::statusFromOpCodeAndChannel(
                                                MidiOpCode::NoteOn, channel),
                                        control),
            MidiOptions(),
            key));
    m_pController->setMapping(m_pMapping);

    // Receive an on/off, sets the control on/off with each press.
    receivedShortMessage(MidiOpCode::NoteOn, channel, control, 0x7F);
    EXPECT_LT(0.0, cpb.get());
    receivedShortMessage(MidiOpCode::NoteOn, channel, control, 0x00);
    EXPECT_DOUBLE_EQ(0.0, cpb.get());

    // Receive an on/off, sets the control on/off with each press.
    receivedShortMessage(MidiOpCode::NoteOn, channel, control, 0x7F);
    EXPECT_LT(0.0, cpb.get());
    receivedShortMessage(MidiOpCode::NoteOn, channel, control, 0x00);
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
    options.setFlag(MidiOption::Button);

    addMapping(MidiInputMapping(MidiKey(MidiUtils::statusFromOpCodeAndChannel(
                                                MidiOpCode::NoteOn, channel),
                                        control),
            options,
            key));
    addMapping(MidiInputMapping(MidiKey(MidiUtils::statusFromOpCodeAndChannel(
                                                MidiOpCode::NoteOff, channel),
                                        control),
            options,
            key));
    m_pController->setMapping(m_pMapping);

    // NOTE(rryan): This behavior is broken!

    // Toggle the switch on, sets the push button on.
    receivedShortMessage(MidiOpCode::NoteOn, channel, control, 0x7F);
    EXPECT_LT(0.0, cpb.get());

    // The push button is stuck down here!

    // Toggle the switch off, sets the push button off.
    receivedShortMessage(MidiOpCode::NoteOff, channel, control, 0x00);
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
    options.setFlag(MidiOption::Switch);

    addMapping(MidiInputMapping(MidiKey(MidiUtils::statusFromOpCodeAndChannel(
                                                MidiOpCode::NoteOn, channel),
                                        control),
            options,
            key));
    addMapping(MidiInputMapping(MidiKey(MidiUtils::statusFromOpCodeAndChannel(
                                                MidiOpCode::NoteOff, channel),
                                        control),
            options,
            key));
    m_pController->setMapping(m_pMapping);

    // NOTE(rryan): This behavior is broken!

    // Toggle the switch on, sets the push button on.
    receivedShortMessage(MidiOpCode::NoteOn, channel, control, 0x7F);
    EXPECT_LT(0.0, cpb.get());

    // The push button is stuck down here!

    // Toggle the switch off, sets the push button on again.
    receivedShortMessage(MidiOpCode::NoteOff, channel, control, 0x00);
    EXPECT_LT(0.0, cpb.get());

    // NOTE(rryan): What is supposed to happen in this case? It's an open
    // question I think. I think if you want to connect a switch MIDI control to
    // a push button CO then the switch should directly set the CO. After all,
    // the mapping author asked for the switch to be interpreted as a switch. If
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

    addMapping(MidiInputMapping(
            MidiKey(MidiUtils::statusFromOpCodeAndChannel(
                            MidiOpCode::ControlChange, channel),
                    control),
            MidiOptions(),
            key));
    m_pController->setMapping(m_pMapping);

    // Receive an on/off, sets the control on/off with each press.
    receivedShortMessage(MidiOpCode::ControlChange, channel, control, 0x7F);
    EXPECT_LT(0.0, cpb.get());
    receivedShortMessage(MidiOpCode::ControlChange, channel, control, 0x00);
    EXPECT_DOUBLE_EQ(0.0, cpb.get());

    // Receive an on/off, sets the control on/off with each press.
    receivedShortMessage(MidiOpCode::ControlChange, channel, control, 0x7F);
    EXPECT_LT(0.0, cpb.get());
    receivedShortMessage(MidiOpCode::ControlChange, channel, control, 0x00);
    EXPECT_DOUBLE_EQ(0.0, cpb.get());
}

TEST_F(MidiControllerTest, ReceiveMessage_ToggleCO_PushOnOff) {
    // Most MIDI controller send push-buttons as (NOTE_ON, 0x7F) for press and
    // (NOTE_OFF, 0x00) for release.
    ConfigKey key("[Channel1]", "keylock");
    ControlPushButton cpb(key);
    cpb.setButtonMode(mixxx::control::ButtonMode::Toggle);

    unsigned char channel = 0x01;
    unsigned char control = 0x10;

    addMapping(MidiInputMapping(MidiKey(MidiUtils::statusFromOpCodeAndChannel(
                                                MidiOpCode::NoteOn, channel),
                                        control),
            MidiOptions(),
            key));
    addMapping(MidiInputMapping(MidiKey(MidiUtils::statusFromOpCodeAndChannel(
                                                MidiOpCode::NoteOff, channel),
                                        control),
            MidiOptions(),
            key));
    m_pController->setMapping(m_pMapping);

    // Receive an on/off, toggles the control.
    receivedShortMessage(MidiOpCode::NoteOn, channel, control, 0x7F);
    receivedShortMessage(MidiOpCode::NoteOff, channel, control, 0x00);

    EXPECT_LT(0.0, cpb.get());

    // Receive an on/off, toggles the control.
    receivedShortMessage(MidiOpCode::NoteOn, channel, control, 0x7F);
    receivedShortMessage(MidiOpCode::NoteOff, channel, control, 0x00);

    EXPECT_DOUBLE_EQ(0.0, cpb.get());
}

TEST_F(MidiControllerTest, ReceiveMessage_ToggleCO_PushOnOn) {
    // Some MIDI controllers send push-buttons as (NOTE_ON, 0x7f) for press and
    // (NOTE_ON, 0x00) for release.
    ConfigKey key("[Channel1]", "keylock");
    ControlPushButton cpb(key);
    cpb.setButtonMode(mixxx::control::ButtonMode::Toggle);

    unsigned char channel = 0x01;
    unsigned char control = 0x10;

    addMapping(MidiInputMapping(MidiKey(MidiUtils::statusFromOpCodeAndChannel(
                                                MidiOpCode::NoteOn, channel),
                                        control),
            MidiOptions(),
            key));
    m_pController->setMapping(m_pMapping);

    // Receive an on/off, toggles the control.
    receivedShortMessage(MidiOpCode::NoteOn, channel, control, 0x7F);
    receivedShortMessage(MidiOpCode::NoteOn, channel, control, 0x00);

    EXPECT_LT(0.0, cpb.get());

    // Receive an on/off, toggles the control.
    receivedShortMessage(MidiOpCode::NoteOn, channel, control, 0x7F);
    receivedShortMessage(MidiOpCode::NoteOn, channel, control, 0x00);

    EXPECT_DOUBLE_EQ(0.0, cpb.get());
}

TEST_F(MidiControllerTest, ReceiveMessage_ToggleCO_ToggleOnOff_ButtonMidiOption) {
    // Using the button MIDI option allows you to use a MIDI toggle button as a
    // push button.
    ConfigKey key("[Channel1]", "keylock");
    ControlPushButton cpb(key);
    cpb.setButtonMode(mixxx::control::ButtonMode::Toggle);

    unsigned char channel = 0x01;
    unsigned char control = 0x10;

    MidiOptions options;
    options.setFlag(MidiOption::Button);

    addMapping(MidiInputMapping(MidiKey(MidiUtils::statusFromOpCodeAndChannel(
                                                MidiOpCode::NoteOn, channel),
                                        control),
            options,
            key));
    addMapping(MidiInputMapping(MidiKey(MidiUtils::statusFromOpCodeAndChannel(
                                                MidiOpCode::NoteOff, channel),
                                        control),
            options,
            key));
    m_pController->setMapping(m_pMapping);

    // NOTE(rryan): If the intended behavior of the button MIDI option is to
    // make a toggle MIDI button act like a push button then this isn't
    // working. The toggle on toggles the CO but the toggle off does nothing.

    // Toggle the switch on, since it is interpreted as a button press it
    // toggles the button on.
    receivedShortMessage(MidiOpCode::NoteOn, channel, control, 0x7F);
    EXPECT_LT(0.0, cpb.get());

    // Toggle the switch off, since it is interpreted as a button release it
    // does nothing to the toggle button.
    receivedShortMessage(MidiOpCode::NoteOff, channel, control, 0x00);
    EXPECT_LT(0.0, cpb.get());
}

TEST_F(MidiControllerTest, ReceiveMessage_ToggleCO_ToggleOnOff_SwitchMidiOption) {
    // Using the switch MIDI option interprets a MIDI toggle button as a toggle
    // button rather than a momentary push button.
    ConfigKey key("[Channel1]", "keylock");
    ControlPushButton cpb(key);
    cpb.setButtonMode(mixxx::control::ButtonMode::Toggle);

    unsigned char channel = 0x01;
    unsigned char control = 0x10;

    MidiOptions options;
    options.setFlag(MidiOption::Switch);

    addMapping(MidiInputMapping(MidiKey(MidiUtils::statusFromOpCodeAndChannel(
                                                MidiOpCode::NoteOn, channel),
                                        control),
            options,
            key));
    addMapping(MidiInputMapping(MidiKey(MidiUtils::statusFromOpCodeAndChannel(
                                                MidiOpCode::NoteOff, channel),
                                        control),
            options,
            key));
    m_pController->setMapping(m_pMapping);

    // NOTE(rryan): If the intended behavior of switch MIDI option is to make a
    // toggle MIDI button act like a toggle button then this isn't working. The
    // toggle on presses the CO and the toggle off presses the CO. This toggles
    // the control but allows it to get out of sync.

    // Toggle the switch on, since it is interpreted as a button press it
    // toggles the control on.
    receivedShortMessage(MidiOpCode::NoteOn, channel, control, 0x7F);
    EXPECT_LT(0.0, cpb.get());

    // Toggle the switch off, since it is interpreted as a button press it
    // toggles the control off.
    receivedShortMessage(MidiOpCode::NoteOff, channel, control, 0x00);
    EXPECT_DOUBLE_EQ(0.0, cpb.get());

    // Meanwhile, the GUI toggles the control on again.
    // NOTE(rryan): Now the MIDI toggle button is out of sync with the toggle
    // CO.
    cpb.set(1.0);

    // Toggle the switch on, since it is interpreted as a button press it
    // toggles the control off (since it was on).
    receivedShortMessage(MidiOpCode::NoteOn, channel, control, 0x7F);
    EXPECT_DOUBLE_EQ(0.0, cpb.get());

    // Toggle the switch off, since it is interpreted as a button press it
    // toggles the control on (since it was off).
    receivedShortMessage(MidiOpCode::NoteOff, channel, control, 0x00);
    EXPECT_LT(0.0, cpb.get());
}

TEST_F(MidiControllerTest, ReceiveMessage_ToggleCO_PushCC) {
    // Some MIDI controllers (e.g. Korg nanoKONTROL) send momentary push-buttons
    // as (CC, 0x7f) for press and (CC, 0x00) for release.
    ConfigKey key("[Channel1]", "keylock");
    ControlPushButton cpb(key);
    cpb.setButtonMode(mixxx::control::ButtonMode::Toggle);

    unsigned char channel = 0x01;
    unsigned char control = 0x10;

    addMapping(MidiInputMapping(
            MidiKey(MidiUtils::statusFromOpCodeAndChannel(
                            MidiOpCode::ControlChange, channel),
                    control),
            MidiOptions(),
            key));
    m_pController->setMapping(m_pMapping);

    // Receive an on/off, toggles the control.
    receivedShortMessage(MidiOpCode::ControlChange, channel, control, 0x7F);
    receivedShortMessage(MidiOpCode::ControlChange, channel, control, 0x00);

    EXPECT_LT(0.0, cpb.get());

    // Receive an on/off, toggles the control.
    receivedShortMessage(MidiOpCode::ControlChange, channel, control, 0x7F);
    receivedShortMessage(MidiOpCode::ControlChange, channel, control, 0x00);

    EXPECT_DOUBLE_EQ(0.0, cpb.get());
}

TEST_F(MidiControllerTest, ReceiveMessage_PotMeterCO_7BitCC) {
    ConfigKey key("[Channel1]", "playposition");

    constexpr double kMinValue = -1234.5;
    constexpr double kMaxValue = 678.9;
    constexpr double kMiddleValue = (kMinValue + kMaxValue) * 0.5;
    ControlPotmeter potmeter(key, kMinValue, kMaxValue);

    unsigned char channel = 0x01;
    unsigned char control = 0x10;

    addMapping(MidiInputMapping(
            MidiKey(MidiUtils::statusFromOpCodeAndChannel(
                            MidiOpCode::ControlChange, channel),
                    control),
            MidiOptions(),
            key));
    m_pController->setMapping(m_pMapping);

    // Receive a 0, MIDI parameter should map to the min value.
    receivedShortMessage(MidiOpCode::ControlChange, channel, control, 0x00);
    EXPECT_DOUBLE_EQ(kMinValue, potmeter.get());

    // Receive a 0x7F, MIDI parameter should map to the potmeter max value.
    receivedShortMessage(MidiOpCode::ControlChange, channel, control, 0x7F);
    EXPECT_DOUBLE_EQ(kMaxValue, potmeter.get());

    // Receive a 0x40, MIDI parameter should map to the potmeter middle value.
    receivedShortMessage(MidiOpCode::ControlChange, channel, control, 0x40);
    EXPECT_DOUBLE_EQ(kMiddleValue, potmeter.get());
}

TEST_F(MidiControllerTest, ReceiveMessage_PotMeterCO_14BitCC) {
    ConfigKey key("[Channel1]", "playposition");

    constexpr double kMinValue = -1234.5;
    constexpr double kMaxValue = 678.9;
    constexpr double kMiddleValue = (kMinValue + kMaxValue) * 0.5;
    ControlPotmeter potmeter(key, kMinValue, kMaxValue);
    potmeter.set(0);

    unsigned char channel = 0x01;
    unsigned char lsb_control = 0x10;
    unsigned char msb_control = 0x11;

    MidiOptions lsb;
    lsb.setFlag(MidiOption::FourteenBitLSB);

    MidiOptions msb;
    msb.setFlag(MidiOption::FourteenBitMSB);

    addMapping(MidiInputMapping(
            MidiKey(MidiUtils::statusFromOpCodeAndChannel(
                            MidiOpCode::ControlChange, channel),
                    lsb_control),
            lsb,
            key));
    addMapping(MidiInputMapping(
            MidiKey(MidiUtils::statusFromOpCodeAndChannel(
                            MidiOpCode::ControlChange, channel),
                    msb_control),
            msb,
            key));
    m_pController->setMapping(m_pMapping);

    // If kMinValue or kMaxValue are such that the middle value is 0 then the
    // set(0) commands below allow us to hide failures.
    ASSERT_NE(0.0, kMiddleValue);

    // Receive a 0x0000 (lsb-first), MIDI parameter should map to the min value.
    potmeter.set(0);
    receivedShortMessage(MidiOpCode::ControlChange, channel, lsb_control, 0x00);
    receivedShortMessage(MidiOpCode::ControlChange, channel, msb_control, 0x00);
    EXPECT_DOUBLE_EQ(kMinValue, potmeter.get());

    // Receive a 0x0000 (msb-first), MIDI parameter should map to the min value.
    potmeter.set(0);
    receivedShortMessage(MidiOpCode::ControlChange, channel, msb_control, 0x00);
    receivedShortMessage(MidiOpCode::ControlChange, channel, lsb_control, 0x00);
    EXPECT_DOUBLE_EQ(kMinValue, potmeter.get());

    // Receive a 0x3FFF (lsb-first), MIDI parameter should map to the max value.
    potmeter.set(0);
    receivedShortMessage(MidiOpCode::ControlChange, channel, lsb_control, 0x7F);
    receivedShortMessage(MidiOpCode::ControlChange, channel, msb_control, 0x7F);
    EXPECT_DOUBLE_EQ(kMaxValue, potmeter.get());

    // Receive a 0x3FFF (msb-first), MIDI parameter should map to the max value.
    potmeter.set(0);
    receivedShortMessage(MidiOpCode::ControlChange, channel, msb_control, 0x7F);
    receivedShortMessage(MidiOpCode::ControlChange, channel, lsb_control, 0x7F);
    EXPECT_DOUBLE_EQ(kMaxValue, potmeter.get());

    // Receive a 0x2000 (lsb-first), MIDI parameter should map to the middle
    // value.
    potmeter.set(0);
    receivedShortMessage(MidiOpCode::ControlChange, channel, lsb_control, 0x00);
    receivedShortMessage(MidiOpCode::ControlChange, channel, msb_control, 0x40);
    EXPECT_DOUBLE_EQ(kMiddleValue, potmeter.get());

    // Receive a 0x2000 (msb-first), MIDI parameter should map to the middle
    // value.
    potmeter.set(0);
    receivedShortMessage(MidiOpCode::ControlChange, channel, msb_control, 0x40);
    receivedShortMessage(MidiOpCode::ControlChange, channel, lsb_control, 0x00);
    EXPECT_DOUBLE_EQ(kMiddleValue, potmeter.get());

    // Check the 14-bit resolution is actually present. Receive a 0x2001
    // (msb-first), MIDI parameter should map to the middle value plus a tiny
    // amount. Scaling is not quite linear for MIDI parameters so just check
    // that incrementing the LSB by 1 is greater than the middle value.
    potmeter.set(0);
    receivedShortMessage(MidiOpCode::ControlChange, channel, msb_control, 0x40);
    receivedShortMessage(MidiOpCode::ControlChange, channel, lsb_control, 0x01);
    EXPECT_LT(kMiddleValue, potmeter.get());

    // Check the 14-bit resolution is actually present. Receive a 0x2001
    // (lsb-first), MIDI parameter should map to the middle value plus a tiny
    // amount. Scaling is not quite linear for MIDI parameters so just check
    // that incrementing the LSB by 1 is greater than the middle value.
    potmeter.set(0);
    receivedShortMessage(MidiOpCode::ControlChange, channel, lsb_control, 0x01);
    receivedShortMessage(MidiOpCode::ControlChange, channel, msb_control, 0x40);
    EXPECT_LT(kMiddleValue, potmeter.get());
}

TEST_F(MidiControllerTest, ReceiveMessage_PotMeterCO_14BitPitchBend) {
    ConfigKey key("[Channel1]", "rate");

    constexpr double kMinValue = -1234.5;
    constexpr double kMaxValue = 678.9;
    constexpr double kMiddleValue = (kMinValue + kMaxValue) * 0.5;
    ControlPotmeter potmeter(key, kMinValue, kMaxValue);
    unsigned char channel = 0x01;

    // The control is ignored in mappings for messages where the control is part
    // of the payload.
    addMapping(MidiInputMapping(
            MidiKey(MidiUtils::statusFromOpCodeAndChannel(
                            MidiOpCode::PitchBendChange, channel),
                    0xFF),
            MidiOptions(),
            key));
    m_pController->setMapping(m_pMapping);

    // Receive a 0x0000, MIDI parameter should map to the min value.
    receivedShortMessage(MidiOpCode::PitchBendChange, channel, 0x00, 0x00);
    EXPECT_DOUBLE_EQ(kMinValue, potmeter.get());

    // Receive a 0x3FFF, MIDI parameter should map to the potmeter max value.
    receivedShortMessage(MidiOpCode::PitchBendChange, channel, 0x7F, 0x7F);
    EXPECT_DOUBLE_EQ(kMaxValue, potmeter.get());

    // Receive a 0x2000, MIDI parameter should map to the potmeter middle value.
    receivedShortMessage(MidiOpCode::PitchBendChange, channel, 0x00, 0x40);
    EXPECT_DOUBLE_EQ(kMiddleValue, potmeter.get());

    // Check the 14-bit resolution is actually present. Receive a 0x2001, MIDI
    // parameter should map to the middle value plus a tiny amount. Scaling is
    // not quite linear for MIDI parameters so just check that incrementing the
    // LSB by 1 is greater than the middle value.
    receivedShortMessage(MidiOpCode::PitchBendChange, channel, 0x01, 0x40);
    EXPECT_LT(kMiddleValue, potmeter.get());
}

TEST_F(MidiControllerTest, JSInputHandler_BindHandler) {
    constexpr double kMinValue = -1234.5;
    constexpr double kMaxValue = 678.9;
    ControlPotmeter potmeter(ConfigKey("[Channel1]", "test_pot"), kMinValue, kMaxValue);
    m_pController->setMapping(m_pMapping);
    EXPECT_EQ(getInputMappingCount(), 0);
    evaluateAndAssert(
            "midi.makeInputHandler(0x90, 0x43, (channel, control, value, status) => {"
            "engine.setParameter('[Channel1]', 'test_pot', value);"
            "})");
    EXPECT_EQ(getInputMappingCount(), 1);
    receivedShortMessage(0x90, 0x43, 0x00);
    EXPECT_DOUBLE_EQ(potmeter.get(), kMinValue);
    receivedShortMessage(0x90, 0x43, 0x7F);
    EXPECT_DOUBLE_EQ(potmeter.get(), kMaxValue);
}

TEST_F(MidiControllerTest, JSInputHandler_ControllerShutdownSlot) {
    m_pController->setMapping(m_pMapping);
    EXPECT_EQ(getInputMappingCount(), 0);
    evaluateAndAssert(
            "midi.makeInputHandler(0x90, 0x43, (channel, control, value, status) => {})");
    EXPECT_EQ(getInputMappingCount(), 1);
    shutdownController();
    EXPECT_EQ(getInputMappingCount(), 0);
}

TEST_F(MidiControllerTest, JSInputHandler_ErrorWhenControlIsTooLarge) {
    m_pController->setMapping(m_pMapping);
    EXPECT_EQ(getInputMappingCount(), 0);
    bool isError = evaluateAndAssert(
            "midi.makeInputHandler(0x90, 0x80, (channel, control, value, status) => {})");
    ASSERT_TRUE(isError);
    EXPECT_EQ(getInputMappingCount(), 0);
}

TEST_F(MidiControllerTest, JSInputHandler_ErrorWhenStatusIsTooSmall) {
    m_pController->setMapping(m_pMapping);
    EXPECT_EQ(getInputMappingCount(), 0);
    bool isError = evaluateAndAssert(
            "midi.makeInputHandler(0x7F, 0x00, (channel, control, value, status) => {})");
    ASSERT_TRUE(isError);
    EXPECT_EQ(getInputMappingCount(), 0);
}
