#include "blemidicontroller.h"

#include "controllers/midi/midioutputhandler.h"
#include "moc_blemidicontroller.cpp"
#include "util/logger.h"

#ifdef Q_OS_ANDROID
#include <QtCore/private/qandroidextras_p.h>

#include <QJniEnvironment>
#include <QJniObject>
#endif

namespace {
const mixxx::Logger kLogger("BleMidiController");

// Standard BLE MIDI Service and Characteristic UUIDs
constexpr const char* kBleMidiServiceUuid = "03B80E5A-EDE8-4B33-A751-6CE34EC4C700";
constexpr const char* kBleMidiCharacteristicUuid = "7772E5DB-3868-4112-A1C9-F2669D106BF3";

constexpr const char* kBleMidiClass = "org/mixxx/BleMidiController";

// Singleton pointer for JNI callbacks
BleMidiController* s_pInstance = nullptr;
} // namespace

BleMidiController::BleMidiController(const QString& deviceName,
        const QString& deviceAddress)
        : MidiController(deviceName),
          m_deviceAddress(deviceAddress),
          m_connected(false),
          m_midiBufferIndex(0),
          m_inSysex(false) {
    // Extract product info from device name
    m_productString = deviceName;
    m_serialNumber = deviceAddress;

    // Try to detect Pioneer DDJ-FLX4 from the name
    if (deviceName.contains("DDJ-FLX4", Qt::CaseInsensitive)) {
        m_vendorString = "AlphaTheta Corporation";
        m_vendorId = 0x2B73;
        m_productId = 0x0045;
    } else if (deviceName.contains("DDJ-400", Qt::CaseInsensitive)) {
        m_vendorString = "AlphaTheta Corporation";
        m_vendorId = 0x2B73;
        m_productId = 0x0043;
    } else if (deviceName.contains("DDJ-FLX2", Qt::CaseInsensitive)) {
        m_vendorString = "AlphaTheta Corporation";
        m_vendorId = 0x2B73;
        m_productId = 0x0049;
    }

    setInputDevice(true);
    setOutputDevice(true);
}

BleMidiController::~BleMidiController() {
    if (s_pInstance == this) {
        s_pInstance = nullptr;
    }
}

int BleMidiController::open(const QString& resourcePath) {
    kLogger.info() << "Opening BLE MIDI device" << getName();

#ifdef Q_OS_ANDROID
    QJniObject activity = QJniObject::callStaticObjectMethod(
            "org/qtproject/qt/android/QtNative",
            "activity",
            "()Landroid/app/Activity;");
    if (!activity.isValid()) {
        kLogger.warning() << "BLE: activity not available";
        return -1;
    }

    // Register this instance for JNI callbacks
    s_pInstance = this;

    // Call Java to connect GATT
    QJniObject addressObj = QJniObject::fromString(m_deviceAddress);
    QJniObject serviceUuidObj = QJniObject::fromString(QString(kBleMidiServiceUuid));
    QJniObject charUuidObj = QJniObject::fromString(QString(kBleMidiCharacteristicUuid));

    jboolean result = QJniObject::callStaticMethod<jboolean>(
            kBleMidiClass,
            "connect",
            "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z",
            activity.object<jobject>(),
            addressObj.object<jstring>(),
            serviceUuidObj.object<jstring>(),
            charUuidObj.object<jstring>());

    if (!result) {
        kLogger.warning() << "BLE: failed to connect GATT to" << m_deviceAddress;
        s_pInstance = nullptr;
        return -1;
    }

    kLogger.info() << "BLE: GATT connection initiated to" << m_deviceAddress;
#endif

    startEngine();
    setOpen(true);
    return 0;
}

int BleMidiController::close() {
    kLogger.info() << "Closing BLE MIDI device" << getName();

#ifdef Q_OS_ANDROID
    QJniObject::callStaticMethod<void>(
            kBleMidiClass,
            "disconnect",
            "()V");
    if (s_pInstance == this) {
        s_pInstance = nullptr;
    }
#endif

    stopEngine();
    m_connected = false;
    setOpen(false);
    return 0;
}

bool BleMidiController::connectDevice() {
#ifdef Q_OS_ANDROID
    QJniObject activity = QJniObject::callStaticObjectMethod(
            "org/qtproject/qt/android/QtNative",
            "activity",
            "()Landroid/app/Activity;");
    if (!activity.isValid()) {
        return false;
    }

    QJniObject addressObj = QJniObject::fromString(m_deviceAddress);
    QJniObject serviceUuidObj = QJniObject::fromString(QString(kBleMidiServiceUuid));
    QJniObject charUuidObj = QJniObject::fromString(QString(kBleMidiCharacteristicUuid));

    jboolean result = QJniObject::callStaticMethod<jboolean>(
            kBleMidiClass,
            "connect",
            "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z",
            activity.object<jobject>(),
            addressObj.object<jstring>(),
            serviceUuidObj.object<jstring>(),
            charUuidObj.object<jstring>());

    m_connected = result;
    return result;
#else
    Q_UNUSED(m_deviceAddress);
    return false;
#endif
}

void BleMidiController::disconnectDevice() {
#ifdef Q_OS_ANDROID
    QJniObject::callStaticMethod<void>(
            kBleMidiClass,
            "disconnect",
            "()V");
#endif
    m_connected = false;
}

void BleMidiController::onMidiDataReceived(const QByteArray& data) {
    if (!s_pInstance)
        return;

    // Parse BLE MIDI data
    // BLE MIDI format: each packet starts with a timestamp byte (high bit set),
    // followed by timestamp-low byte and then MIDI messages.
    // See: https://www.midi.org/specifications/item/bluetooth-le-midi
    const char* pData = data.constData();
    int len = data.size();

    if (len < 2)
        return;

    int offset = 0;

    // Skip timestamp header bytes
    // First byte: timestamp_high (bit 7 = 1)
    // Second byte: timestamp_low (bit 7 = 1)
    if (offset < len && (pData[offset] & 0x80)) {
        offset++; // timestamp_high
    }
    if (offset < len && (pData[offset] & 0x80)) {
        offset++; // timestamp_low
    }

    // Process remaining MIDI bytes
    while (offset < len) {
        unsigned char byte = pData[offset];

        if (byte == 0xF0) {
            // SysEx start
            s_pInstance->m_inSysex = true;
            s_pInstance->m_midiBuffer[0] = byte;
            s_pInstance->m_midiBufferIndex = 1;
            offset++;
        } else if (byte == 0xF7 && s_pInstance->m_inSysex) {
            // SysEx end
            if (s_pInstance->m_midiBufferIndex < 1024) {
                s_pInstance->m_midiBuffer[s_pInstance->m_midiBufferIndex++] = byte;
            }
            // Process complete SysEx
            QByteArray sysexData(
                    reinterpret_cast<const char*>(s_pInstance->m_midiBuffer),
                    s_pInstance->m_midiBufferIndex);
            s_pInstance->receive(sysexData, mixxx::Duration());
            s_pInstance->m_inSysex = false;
            s_pInstance->m_midiBufferIndex = 0;
            offset++;
        } else if (s_pInstance->m_inSysex) {
            // SysEx data byte - check for timestamp
            if ((byte & 0x80) && offset + 1 < len && (pData[offset + 1] & 0x80)) {
                // This is a timestamp byte pair, skip it
                offset += 2;
            } else {
                if (s_pInstance->m_midiBufferIndex < 1024) {
                    s_pInstance->m_midiBuffer[s_pInstance->m_midiBufferIndex++] = byte;
                }
                offset++;
            }
        } else if (byte & 0x80) {
            // Status byte - check for timestamp
            if (offset + 1 < len && (pData[offset + 1] & 0x80)) {
                // Next byte is also high-bit = timestamp, skip this timestamp pair
                offset += 2;
                if (offset >= len)
                    break;
                byte = pData[offset];
                if (!(byte & 0x80))
                    break; // Not a status byte after timestamp
            }

            unsigned char status = byte;
            unsigned char type = status & 0xF0;

            if (type == 0xC0 || type == 0xD0) {
                // 2-byte message (Program Change, Channel Pressure)
                if (offset + 1 < len) {
                    unsigned char data1 = pData[offset + 1];
                    if (data1 & 0x80) {
                        // Timestamp, try to skip
                        offset++;
                        continue;
                    }
                    s_pInstance->receive(QByteArray(1, status) +
                                    QByteArray(1, data1),
                            mixxx::Duration());
                    offset += 2;
                } else {
                    offset++;
                }
            } else if (status == 0xF1 || status == 0xF3) {
                // 2-byte system common
                if (offset + 1 < len) {
                    unsigned char data1 = pData[offset + 1];
                    s_pInstance->receive(QByteArray(1, status) +
                                    QByteArray(1, data1),
                            mixxx::Duration());
                    offset += 2;
                } else {
                    offset++;
                }
            } else if (type == 0x80 || type == 0x90 || type == 0xA0 ||
                    type == 0xB0 || type == 0xE0) {
                // 3-byte message (Note On/Off, CC, etc.)
                if (offset + 2 < len) {
                    unsigned char data1 = pData[offset + 1];
                    unsigned char data2 = pData[offset + 2];
                    if (data1 & 0x80 || data2 & 0x80) {
                        // Timestamp interference, skip byte by byte
                        offset++;
                        continue;
                    }
                    s_pInstance->receive(QByteArray(1, status) +
                                    QByteArray(1, data1) +
                                    QByteArray(1, data2),
                            mixxx::Duration());
                    offset += 3;
                } else {
                    offset++;
                }
            } else {
                // Unknown or single-byte message
                offset++;
            }
        } else {
            // Data byte without status - skip (running status not handled here)
            offset++;
        }
    }
}

bool BleMidiController::poll() {
    // BLE MIDI uses callbacks, not polling
    return false;
}

void BleMidiController::sendShortMsg(unsigned char status,
        unsigned char byte1,
        unsigned char byte2) {
#ifdef Q_OS_ANDROID
    // BLE MIDI write: wrap in timestamp header
    // Format: [timestamp_high | timestamp_low] [status] [data1] [data2]
    QByteArray packet;
    // Timestamp: use current millisecond count
    qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    unsigned char tsHigh = 0x80 | ((nowMs >> 7) & 0x3F);
    unsigned char tsLow = 0x80 | (nowMs & 0x7F);
    packet.append(tsHigh);
    packet.append(tsLow);
    packet.append(status);
    packet.append(byte1);
    packet.append(byte2);

    QJniObject jData = QJniObject::fromString(QString(packet.toHex()));
    QJniObject::callStaticMethod<void>(
            kBleMidiClass,
            "writeMidiData",
            "(Ljava/lang/String;)V",
            jData.object<jstring>());
#endif
    Q_UNUSED(status);
    Q_UNUSED(byte1);
    Q_UNUSED(byte2);
}

bool BleMidiController::sendBytes(const QByteArray& data) {
#ifdef Q_OS_ANDROID
    QByteArray packet;
    qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    unsigned char tsHigh = 0x80 | ((nowMs >> 7) & 0x3F);
    unsigned char tsLow = 0x80 | (nowMs & 0x7F);
    packet.append(tsHigh);
    packet.append(tsLow);
    packet.append(data);

    QJniObject jData = QJniObject::fromString(QString(packet.toHex()));
    QJniObject::callStaticMethod<void>(
            kBleMidiClass,
            "writeMidiData",
            "(Ljava/lang/String;)V",
            jData.object<jstring>());
    return true;
#else
    return false;
#endif
}
