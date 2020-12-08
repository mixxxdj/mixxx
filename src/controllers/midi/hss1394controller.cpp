#include "controllers/midi/hss1394controller.h"

#include "controllers/controllerdebug.h"
#include "controllers/midi/midiutils.h"
#include "moc_hss1394controller.cpp"
#include "util/time.h"

DeviceChannelListener::DeviceChannelListener(QObject* pParent, QString name)
        : QObject(pParent),
          hss1394::ChannelListener(),
          m_sName(name) {
}

DeviceChannelListener::~DeviceChannelListener() {
}

void DeviceChannelListener::Process(const hss1394::uint8 *pBuffer, hss1394::uint uBufferSize) {
    unsigned int i = 0;

    mixxx::Duration timestamp = mixxx::Time::elapsed();

    // If multiple three-byte messages arrive right next to each other, handle them all
    while (i < uBufferSize) {
        unsigned char status = pBuffer[i];
        unsigned char opcode = status & 0xF0;
        unsigned char channel = status & 0x0F;
        unsigned char note;
        unsigned char velocity;
        switch (status & 0xF0) {
            case MIDI_NOTE_OFF:
            case MIDI_NOTE_ON:
            case MIDI_AFTERTOUCH:
            case MIDI_CC:
            case MIDI_PITCH_BEND:
                if (i + 2 < uBufferSize) {
                    note = pBuffer[i+1];
                    velocity = pBuffer[i+2];
                    emit incomingData(status, note, velocity, timestamp);
                } else {
                    qWarning() << "Buffer underflow in DeviceChannelListener::Process()";
                }
                i += 3;
                break;
            default:
                // Handle platter messages and any others that are not 3 bytes
                QByteArray outArray((char*)pBuffer,uBufferSize);
                emit incomingData(outArray, timestamp);
                i = uBufferSize;
                break;
        }
    }
}

void DeviceChannelListener::Disconnected() {
    qDebug() << "HSS1394 device" << m_sName << "disconnected";
}

void DeviceChannelListener::Reconnected() {
    qDebug() << "HSS1394 device" << m_sName << "re-connected";
}

Hss1394Controller::Hss1394Controller(
        const hss1394::TNodeInfo& deviceInfo,
        int deviceIndex,
        UserSettingsPointer pConfig)
        : MidiController(),
          m_deviceInfo(deviceInfo),
          m_iDeviceIndex(deviceIndex) {
    // Note: We prepend the input stream's index to the device's name to prevent
    // duplicate devices from causing mayhem.
    //setDeviceName(QString("H%1. %2").arg(QString::number(m_iDeviceIndex), QString(deviceInfo.sName.c_str())));
    setDeviceName(QString("%1").arg(QString(deviceInfo.sName.c_str())));

    // All HSS1394 devices are full-duplex
    setInputDevice(true);
    setOutputDevice(true);
}

Hss1394Controller::~Hss1394Controller() {
    if (isOpen()) {
        close();
    }
}

int Hss1394Controller::open() {
    if (isOpen()) {
        qDebug() << "HSS1394 device" << getName() << "already open";
        return -1;
    }

    if (getName() == MIXXX_HSS1394_NO_DEVICE_STRING) {
        return -1;
    }

    controllerDebug("Hss1394Controller: Opening" << getName() << "index"
                    << m_iDeviceIndex);

    using namespace hss1394;

    m_pChannel = Node::Instance()->OpenChannel(m_iDeviceIndex);
    if (m_pChannel == NULL) {
        qDebug() << "HSS1394 device" << getName() << "could not be opened";
        m_pChannelListener = NULL;
        return -1;
    }

    m_pChannelListener = new DeviceChannelListener(this, getName());
    connect(m_pChannelListener,
            QOverload<unsigned char,
                    unsigned char,
                    unsigned char,
                    mixxx::Duration>::of(&DeviceChannelListener::incomingData),
            this,
            QOverload<unsigned char,
                    unsigned char,
                    unsigned char,
                    mixxx::Duration>::of(&Hss1394Controller::receive));
    connect(m_pChannelListener,
            QOverload<const QByteArray&, mixxx::Duration>::of(&DeviceChannelListener::incomingData),
            this,
            QOverload<const QByteArray&, mixxx::Duration>::of(&Hss1394Controller::receive));

    if (!m_pChannel->InstallChannelListener(m_pChannelListener)) {
        qDebug() << "HSS1394 channel listener could not be installed for device" << getName();
        delete m_pChannelListener;
        m_pChannelListener = NULL;
        m_pChannel = NULL;
    }

    // TODO(XXX): Should be done in script, not in Mixxx
    if (getName().contains("SCS.1d",Qt::CaseInsensitive)) {
        // If we are an SCS.1d, set the record encoder event timer to fire at 1ms intervals
        //  to match the 1ms scratch timer in the controller engine
        //
        // By default on f/w version 1.25, a new record encoder event (one new position)
        //  is sent at 500 Hz max, 2ms. When this event occurs, a second timer is reset.
        //  By default this second timer expires periodically at 60 Hz max, around 16.6ms.

        int iPeriod = 60000/1000;   // 1000Hz = 1ms. (Internal clock is 60kHz.)
        int iTimer = 3; // 3 for new event timer, 4 for second �same position repeated� timer
        if (m_pChannel->SendUserControl(iTimer, (const hss1394::uint8*)&iPeriod, 3) == 0)
            qWarning() << "Unable to set SCS.1d platter timer period.";
    }

    setOpen(true);
    startEngine();
    return 0;
}

int Hss1394Controller::close() {
    if (!isOpen()) {
        qDebug() << "HSS1394 device" << getName() << "already closed";
        return -1;
    }

    disconnect(m_pChannelListener,
            QOverload<unsigned char,
                    unsigned char,
                    unsigned char,
                    mixxx::Duration>::of(&DeviceChannelListener::incomingData),
            this,
            QOverload<unsigned char,
                    unsigned char,
                    unsigned char,
                    mixxx::Duration>::of(&Hss1394Controller::receive));
    disconnect(m_pChannelListener,
            QOverload<const QByteArray&, mixxx::Duration>::of(&DeviceChannelListener::incomingData),
            this,
            QOverload<const QByteArray&, mixxx::Duration>::of(&Hss1394Controller::receive));

    stopEngine();
    MidiController::close();

    // Clean up the HSS1394Node
    using namespace hss1394;
    if (!Node::Instance()->ReleaseChannel(m_pChannel)) {
        qDebug() << "HSS1394 device" << getName() << "could not be released";
        return -1;
    }
    if (m_pChannelListener != NULL) {
        delete m_pChannelListener;
        m_pChannelListener = NULL;
    }

    setOpen(false);
    return 0;
}

void Hss1394Controller::sendShortMsg(unsigned char status, unsigned char byte1,
                                     unsigned char byte2) {
    unsigned char data[3] = { status, byte1, byte2 };

    int bytesSent = m_pChannel->SendChannelBytes(data, 3);
    controllerDebug(MidiUtils::formatMidiMessage(getName(),
                                                 status, byte1, byte2,
                                                 MidiUtils::channelFromStatus(status),
                                                 MidiUtils::opCodeFromStatus(status)));

    //if (bytesSent != 3) {
    //    qDebug()<<"ERROR: Sent" << bytesSent << "of 3 bytes:" << message;
    //    //m_pChannel->Flush();
    //}
}

void Hss1394Controller::sendBytes(const QByteArray& data) {
    int bytesSent = m_pChannel->SendChannelBytes(
        (unsigned char*)data.constData(), data.size());

    controllerDebug(MidiUtils::formatSysexMessage(getName(), data));
    //if (bytesSent != length) {
    //    qDebug()<<"ERROR: Sent" << bytesSent << "of" << length << "bytes (SysEx)";
    //    //m_pChannel->Flush();
    //}
}
