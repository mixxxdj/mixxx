#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QScopedPointer>

#include "controllers/midi/portmidicontroller.h"
#include "controllers/midi/portmididevice.h"
#include "test/mixxxtest.h"

using ::testing::_;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArrayArgument;

class MockPortMidiController : public PortMidiController {
  public:
    MockPortMidiController(const PmDeviceInfo* inputDeviceInfo,
            const PmDeviceInfo* outputDeviceInfo,
            int inputDeviceIndex,
            int outputDeviceIndex)
            : PortMidiController(inputDeviceInfo,
                      outputDeviceInfo,
                      inputDeviceIndex,
                      outputDeviceIndex) {
    }
    ~MockPortMidiController() override {
    }

    void sendShortMsg(unsigned char status, unsigned char byte1, unsigned char byte2) override {
        PortMidiController::sendShortMsg(status, byte1, byte2);
    }

    void sendSysexMsg(const QList<int>& data, unsigned int length) {
        PortMidiController::sendSysexMsg(data, length);
    }

    MOCK_METHOD4(receive, void(unsigned char, unsigned char, unsigned char,
                               mixxx::Duration));
    MOCK_METHOD2(receive, void(const QByteArray&, mixxx::Duration));
};

class MockPortMidiDevice : public PortMidiDevice {
  public:
    MockPortMidiDevice(PmDeviceInfo* info, int index)
            : PortMidiDevice(info, index) {
    }

    MOCK_CONST_METHOD0(isOpen, bool());
    MOCK_METHOD1(openInput, PmError(int32_t));
    MOCK_METHOD0(openOutput, PmError());
    MOCK_METHOD0(close, PmError());
    MOCK_METHOD0(poll, PmError());
    MOCK_METHOD2(read, int(PmEvent*, int32_t));
    MOCK_METHOD1(writeShort, PmError(int32_t));
    MOCK_METHOD1(writeSysEx, PmError(unsigned char*));
};

class PortMidiControllerTest : public MixxxTest {
  protected:
    PortMidiControllerTest()
            : m_mockInput(new MockPortMidiDevice(&m_inputDeviceInfo, 0)),
              m_mockOutput(new MockPortMidiDevice(&m_outputDeviceInfo, 0)) {
        m_inputDeviceInfo.name = "Test Input Device";
        m_inputDeviceInfo.interf = "Test";
        m_inputDeviceInfo.input = 1;
        m_inputDeviceInfo.output = 0;
        m_inputDeviceInfo.opened = 0;

        m_outputDeviceInfo.name = "Test Output Device";
        m_outputDeviceInfo.interf = "Test";
        m_outputDeviceInfo.input = 0;
        m_outputDeviceInfo.output = 1;
        m_outputDeviceInfo.opened = 0;

        m_pController.reset(new MockPortMidiController(
                &m_inputDeviceInfo, &m_outputDeviceInfo, 0, 0));
        m_pController->setPortMidiInputDevice(m_mockInput);
        m_pController->setPortMidiOutputDevice(m_mockOutput);
    }

    void openDevice() {
        m_pController->open();
    }

    void closeDevice() {
        m_pController->close();
    }

    void pollDevice() {
        m_pController->poll();
    }

    PmDeviceInfo m_inputDeviceInfo;
    PmDeviceInfo m_outputDeviceInfo;
    MockPortMidiDevice* m_mockInput;
    MockPortMidiDevice* m_mockOutput;
    QScopedPointer<MockPortMidiController> m_pController;
};

PmEvent MakeEvent(PmMessage message, PmTimestamp timestamp) {
    PmEvent event;
    event.message = message;
    event.timestamp = timestamp;
    return event;
}

MATCHER_P(ByteArrayEquals, value,
          "Checks that the non-NULL terminated argument array exactly equals "
          "the provided byte container.") {
    for (int i = 0; i < value.size(); ++i) {
        if (arg[i] != value.at(i))
            return false;
    }
    return true;
}

TEST_F(PortMidiControllerTest, OpenClose) {
    Sequence input;
    ON_CALL(*m_mockInput, isOpen())
            .WillByDefault(Return(false));
    EXPECT_CALL(*m_mockInput, openInput(MIXXX_PORTMIDI_BUFFER_LEN))
            .InSequence(input)
            .WillOnce(Return(pmNoError));
    EXPECT_CALL(*m_mockInput, isOpen())
            .InSequence(input)
            .WillRepeatedly(Return(true));
    EXPECT_CALL(*m_mockInput, close())
            .InSequence(input)
            .WillOnce(Return(pmNoError));

    Sequence output;
    ON_CALL(*m_mockOutput, isOpen())
            .WillByDefault(Return(false));
    EXPECT_CALL(*m_mockOutput, openOutput())
            .WillOnce(Return(pmNoError));
    EXPECT_CALL(*m_mockOutput, isOpen())
            .InSequence(output)
            .WillRepeatedly(Return(true));
    EXPECT_CALL(*m_mockOutput, close())
            .InSequence(output)
            .WillOnce(Return(pmNoError));

    openDevice();
    EXPECT_TRUE(m_pController->isOpen());
    closeDevice();
    EXPECT_FALSE(m_pController->isOpen());
};

TEST_F(PortMidiControllerTest, WriteShort) {
    // Note that Pm_WriteShort takes an int32_t formatted as 0x00B2B1SS where SS
    // is the status byte, B1 is the first message byte and B2 is the second
    // message byte.
    Sequence output;
    EXPECT_CALL(*m_mockOutput, isOpen())
            .WillRepeatedly(Return(true));
    EXPECT_CALL(*m_mockOutput, writeShort(0x403C90))
            .InSequence(output)
            .WillOnce(Return(pmNoError));
    EXPECT_CALL(*m_mockOutput, writeShort(0xFFFFFF))
            .InSequence(output)
            .WillOnce(Return(pmBadData));
    EXPECT_CALL(*m_mockOutput, writeShort(0x403C80))
            .InSequence(output)
            .WillOnce(Return(pmNoError));

    m_pController->sendShortMsg(0x90, 0x3C, 0x40);
    m_pController->sendShortMsg(0xFF, 0xFF, 0xFF);
    m_pController->sendShortMsg(0x80, 0x3C, 0x40);
};

TEST_F(PortMidiControllerTest, WriteSysex) {
    QList<int> sysex;
    sysex.append(0xF0);
    sysex.append(0x12);
    sysex.append(0xF7);

    EXPECT_CALL(*m_mockOutput, isOpen())
            .WillRepeatedly(Return(true));
    EXPECT_CALL(*m_mockOutput, writeSysEx(ByteArrayEquals(sysex)))
            .WillOnce(Return(pmNoError));
    m_pController->sendSysexMsg(sysex, sysex.length());
};

TEST_F(PortMidiControllerTest, WriteSysex_Malformed) {
    QList<int> sysex;
    sysex.append(0xF0);
    sysex.append(0x12);

    EXPECT_CALL(*m_mockOutput, isOpen())
            .WillRepeatedly(Return(true));
    EXPECT_CALL(*m_mockOutput, writeSysEx(_))
            .Times(0);
    m_pController->sendSysexMsg(sysex, sysex.length());
};

TEST_F(PortMidiControllerTest, Poll_Read_NoInput) {
    Sequence poll;
    EXPECT_CALL(*m_mockInput, isOpen())
            .WillRepeatedly(Return(true));
    EXPECT_CALL(*m_mockInput, poll())
            .InSequence(poll)
            .WillOnce(Return((PmError)FALSE));
    EXPECT_CALL(*m_mockInput, poll())
            .InSequence(poll)
            .WillOnce(Return((PmError)TRUE));
    EXPECT_CALL(*m_mockInput, read(_, _))
            .InSequence(poll)
            .WillOnce(Return(0));

    pollDevice();
    pollDevice();
};

TEST_F(PortMidiControllerTest, Poll_Read_Basic) {
    std::vector<PmEvent> messages;
    messages.push_back(MakeEvent(0x403C90, 0x0));
    messages.push_back(MakeEvent(0x403C80, 0x1));

    Sequence read;
    EXPECT_CALL(*m_mockInput, isOpen())
            .WillRepeatedly(Return(true));
    EXPECT_CALL(*m_mockInput, poll())
            .InSequence(read)
            .WillOnce(Return((PmError)TRUE));
    EXPECT_CALL(*m_mockInput, read(NotNull(), _))
            .InSequence(read)
            .WillOnce(DoAll(SetArrayArgument<0>(messages.begin(), messages.end()),
                            Return(messages.size())));

    EXPECT_CALL(*m_pController, receive(0x90, 0x3C, 0x40, _))
            .InSequence(read);
    EXPECT_CALL(*m_pController, receive(0x80, 0x3C, 0x40, _))
            .InSequence(read);

    pollDevice();
};

TEST_F(PortMidiControllerTest, Poll_Read_SysExWithRealtime) {
    std::vector<PmEvent> messages;
    messages.push_back(MakeEvent(0x332211F0, 0x0));
    messages.push_back(MakeEvent(0x000000F8, 0x1));
    messages.push_back(MakeEvent(0x77665544, 0x0));
    messages.push_back(MakeEvent(0x000000FA, 0x2));
    messages.push_back(MakeEvent(0x000000F7, 0x0));

    QByteArray sysex;
    sysex.append('\xF0');
    sysex.append('\x11');
    sysex.append('\x22');
    sysex.append('\x33');
    sysex.append('\x44');
    sysex.append('\x55');
    sysex.append('\x66');
    sysex.append('\x77');
    sysex.append('\xF7');

    Sequence read;
    EXPECT_CALL(*m_mockInput, isOpen())
            .WillRepeatedly(Return(true));
    EXPECT_CALL(*m_mockInput, poll())
            .InSequence(read)
            .WillOnce(Return((PmError)TRUE));
    EXPECT_CALL(*m_mockInput, read(NotNull(), _))
            .InSequence(read)
            .WillOnce(DoAll(SetArrayArgument<0>(messages.begin(), messages.end()),
                            Return(messages.size())));
    EXPECT_CALL(*m_pController, receive(0xF8, 0x00, 0x00, _))
            .InSequence(read);
    EXPECT_CALL(*m_pController, receive(0xFA, 0x00, 0x00, _))
            .InSequence(read);
    EXPECT_CALL(*m_pController, receive(sysex, _))
            .InSequence(read);

    pollDevice();
};

TEST_F(PortMidiControllerTest, Poll_Read_SysEx) {
    std::vector<PmEvent> messages;
    messages.push_back(MakeEvent(0x332211F0, 0x0));
    messages.push_back(MakeEvent(0xF7665544, 0x1));

    QByteArray sysex;
    sysex.append('\xF0');
    sysex.append('\x11');
    sysex.append('\x22');
    sysex.append('\x33');
    sysex.append('\x44');
    sysex.append('\x55');
    sysex.append('\x66');
    sysex.append('\xF7');

    Sequence read;
    EXPECT_CALL(*m_mockInput, isOpen())
            .WillRepeatedly(Return(true));
    EXPECT_CALL(*m_mockInput, poll())
            .InSequence(read)
            .WillOnce(Return((PmError)TRUE));
    EXPECT_CALL(*m_mockInput, read(NotNull(), _))
            .InSequence(read)
            .WillOnce(DoAll(SetArrayArgument<0>(messages.begin(), messages.end()),
                            Return(messages.size())));
    EXPECT_CALL(*m_pController, receive(sysex, _))
            .InSequence(read);

    pollDevice();
};

TEST_F(PortMidiControllerTest,
       Poll_Read_SysExWithRealtime_CoincidentalRealtimeByte) {
    // We used to incorrectly treat an 0xF8 occurring in a SysEx message as a
    // realtime message. This test verifies that we do not do this anymore.
    std::vector<PmEvent> messages;
    messages.push_back(MakeEvent(0x332211F0, 0x0));
    messages.push_back(MakeEvent(0x6655F844, 0x0));
    messages.push_back(MakeEvent(0x0000F777, 0x0));

    QByteArray sysex;
    sysex.append('\xF0');
    sysex.append('\x11');
    sysex.append('\x22');
    sysex.append('\x33');
    sysex.append('\x44');
    sysex.append('\xF8');
    sysex.append('\x55');
    sysex.append('\x66');
    sysex.append('\x77');
    sysex.append('\xF7');

    Sequence read;
    EXPECT_CALL(*m_mockInput, isOpen())
            .WillRepeatedly(Return(true));
    EXPECT_CALL(*m_mockInput, poll())
            .InSequence(read)
            .WillOnce(Return((PmError)TRUE));
    EXPECT_CALL(*m_mockInput, read(NotNull(), _))
            .InSequence(read)
            .WillOnce(DoAll(SetArrayArgument<0>(messages.begin(), messages.end()),
                            Return(messages.size())));
    EXPECT_CALL(*m_pController, receive(sysex, _))
            .InSequence(read);

    pollDevice();
};

TEST_F(PortMidiControllerTest, Poll_Read_SysExInterrupted_FollowedByNormalMessage) {
    // According to the PortMIDI documentation when a SysEx message is
    // interrupted, we will expect to see a non-realtime status byte as a new
    // message before seeing an EOX terminating SysEx. In this event we drop the
    // SysEx message and process the new message as normal.

    std::vector<PmEvent> messages;
    messages.push_back(MakeEvent(0x332211F0, 0x0));
    messages.push_back(MakeEvent(0x00403C90, 0x0));

    Sequence read;
    EXPECT_CALL(*m_mockInput, isOpen())
            .WillRepeatedly(Return(true));
    EXPECT_CALL(*m_mockInput, poll())
            .InSequence(read)
            .WillOnce(Return((PmError)TRUE));
    EXPECT_CALL(*m_mockInput, read(NotNull(), _))
            .InSequence(read)
            .WillOnce(DoAll(SetArrayArgument<0>(messages.begin(), messages.end()),
                            Return(messages.size())));
    EXPECT_CALL(*m_pController, receive(0x90, 0x3C, 0x40, _))
            .InSequence(read);

    pollDevice();
};

TEST_F(PortMidiControllerTest, Poll_Read_SysExInterrupted_FollowedBySysExMessage) {
    // According to the PortMIDI documentation when a SysEx message is
    // interrupted, we will expect to see a non-realtime status byte as a new
    // message before seeing an EOX terminating SysEx. In this event we drop the
    // SysEx message and process the new message as normal.

    std::vector<PmEvent> messages;
    messages.push_back(MakeEvent(0x332211F0, 0x0));
    messages.push_back(MakeEvent(0x77665544, 0x0));
    messages.push_back(MakeEvent(0x332211F0, 0x1));
    messages.push_back(MakeEvent(0xF7665544, 0x0));

    QByteArray sysex;
    sysex.append('\xF0');
    sysex.append('\x11');
    sysex.append('\x22');
    sysex.append('\x33');
    sysex.append('\x44');
    sysex.append('\x55');
    sysex.append('\x66');
    sysex.append('\xF7');

    Sequence read;
    EXPECT_CALL(*m_mockInput, isOpen())
            .WillRepeatedly(Return(true));
    EXPECT_CALL(*m_mockInput, poll())
            .InSequence(read)
            .WillOnce(Return((PmError)TRUE));
    EXPECT_CALL(*m_mockInput, read(NotNull(), _))
            .InSequence(read)
            .WillOnce(DoAll(SetArrayArgument<0>(messages.begin(), messages.end()),
                            Return(messages.size())));
    EXPECT_CALL(*m_pController, receive(sysex, _))
            .InSequence(read);

    pollDevice();
};


TEST_F(PortMidiControllerTest, Poll_Read_SysEx_BufferOverflow) {
    // According to the PortMIDI documentation when a SysEx message is
    // interrupted, we will expect to see a non-realtime status byte as a new
    // message before seeing an EOX terminating SysEx. In this event we drop the
    // SysEx message and process the new message as normal.

    std::vector<PmEvent> messages1;
    messages1.push_back(MakeEvent(0x332211F0, 0x0));
    messages1.push_back(MakeEvent(0x77665544, 0x0));

    std::vector<PmEvent> messages2;
    messages2.push_back(MakeEvent(0x332211F0, 0x1));

    std::vector<PmEvent> messages3;
    messages3.push_back(MakeEvent(0xF7665544, 0x2));

    QByteArray sysex;
    sysex.append('\xF0');
    sysex.append('\x11');
    sysex.append('\x22');
    sysex.append('\x33');
    sysex.append('\x44');
    sysex.append('\x55');
    sysex.append('\x66');
    sysex.append('\xF7');

    Sequence read;
    EXPECT_CALL(*m_mockInput, isOpen())
            .WillRepeatedly(Return(true));

    // Poll 1 -- returns messages1.
    EXPECT_CALL(*m_mockInput, poll())
            .InSequence(read)
            .WillOnce(Return((PmError)TRUE));
    EXPECT_CALL(*m_mockInput, read(NotNull(), _))
            .InSequence(read)
            .WillOnce(DoAll(SetArrayArgument<0>(messages1.begin(), messages1.end()),
                            Return(messages1.size())));

    // Poll 2 -- buffer overflow.
    EXPECT_CALL(*m_mockInput, poll())
            .InSequence(read)
            .WillOnce(Return((PmError)TRUE));
    EXPECT_CALL(*m_mockInput, read(NotNull(), _))
            .InSequence(read)
            .WillOnce(Return(pmBufferOverflow));

    // Poll 3 -- returns messages2.
    EXPECT_CALL(*m_mockInput, poll())
            .InSequence(read)
            .WillOnce(Return((PmError)TRUE));
    EXPECT_CALL(*m_mockInput, read(NotNull(), _))
            .InSequence(read)
            .WillOnce(DoAll(SetArrayArgument<0>(messages2.begin(), messages2.end()),
                            Return(messages2.size())));

    // Poll 4 -- returns messages3.
    EXPECT_CALL(*m_mockInput, poll())
            .InSequence(read)
            .WillOnce(Return((PmError)TRUE));
    EXPECT_CALL(*m_mockInput, read(NotNull(), _))
            .InSequence(read)
            .WillOnce(DoAll(SetArrayArgument<0>(messages3.begin(), messages3.end()),
                            Return(messages3.size())));
    EXPECT_CALL(*m_pController, receive(sysex, _))
            .InSequence(read);

    pollDevice();
    pollDevice();
    pollDevice();
    pollDevice();
};
