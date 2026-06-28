#include "blemidicontroller.h"

#ifdef Q_OS_ANDROID

#include <QtCore/private/qandroidextras_p.h>

#include <QJniEnvironment>
#include <QJniObject>

#include "controllers/midi/midioutputhandler.h"
#include "moc_blemidicontroller.cpp"
#include "util/logger.h"

namespace {
const mixxx::Logger kLogger("BleMidiController");

// Standard BLE MIDI Service and Characteristic UUIDs
constexpr const char* kBleMidiServiceUuid = "03B80E5A-EDE8-4B33-A751-6CE34EC4C700";
constexpr const char* kBleMidiCharacteristicUuid = "7772E5DB-3868-4112-A1A9-F2669D106BF3";

// JNI class name for BleMidiController.java
constexpr const char* kBleMidiClass = "org/mixxx/BleMidiController";
constexpr const char* kBleScannerClass = "org/mixxx/BleMidiScanner";

} // namespace

BleMidiController* BleMidiController::s_pInstance = nullptr;

BleMidiController::BleMidiController(const QString& deviceName,
        const QString& deviceAddress)
        : MidiController(deviceName),
          m_deviceAddress(deviceAddress),
          m_connected(false) {
    m_vendorString = "Pioneer";
    m_productString = deviceName;
}

BleMidiController::~BleMidiController() {
    if (s_pInstance == this) {
        s_pInstance = nullptr;
    }
}

bool BleMidiController::isPolling() const {
    return false; // We use callbacks, not polling
}

int BleMidiController::open(const QString& resourcePath) {
    Q_UNUSED(resourcePath);
    kLogger.info() << "Opening BLE MIDI device" << getName();

    // Register this instance for JNI callbacks
    s_pInstance = this;

    // Check if already connected by Java side (automatic during scan)
    bool alreadyConnected = QJniObject::callStaticMethod<jboolean>(
            kBleScannerClass,
            "isConnected",
            "()Z");

    if (alreadyConnected) {
        kLogger.info() << "BLE device already connected:" << m_deviceAddress;
        startEngine();
        setOpen(true);
        return 0;
    }

    // Not yet connected - the Java BLE scanner will connect automatically
    // during the scan cycle. We accept the device as "opening" and wait.
    kLogger.info() << "BLE device pending connection:" << m_deviceAddress;
    startEngine();
    setOpen(true);
    return 0;
}

int BleMidiController::close() {
    kLogger.info() << "Closing BLE MIDI device" << getName();

    QJniObject::callStaticMethod<void>(
            kBleScannerClass,
            "disconnectAll",
            "()V");
    if (s_pInstance == this) {
        s_pInstance = nullptr;
    }

    stopEngine();
    m_connected = false;
    setOpen(false);
    return 0;
}

PhysicalTransportProtocol BleMidiController::getPhysicalTransportProtocol() const {
    return PhysicalTransportProtocol::BlueTooth;
}

QString BleMidiController::getVendorString() const {
    return m_vendorString;
}

QString BleMidiController::getProductString() const {
    return m_productString;
}

std::optional<uint16_t> BleMidiController::getVendorId() const {
    // DDJ-FLX4 USB vendor ID
    return 0x2B73;
}

std::optional<uint16_t> BleMidiController::getProductId() const {
    // DDJ-FLX4 USB product ID
    return 0x0045;
}

QString BleMidiController::getSerialNumber() const {
    return m_serialNumber;
}

std::optional<uint8_t> BleMidiController::getUsbInterfaceNumber() const {
    return std::nullopt;
}

bool BleMidiController::connectDevice() {
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
}

void BleMidiController::disconnectDevice() {
    QJniObject::callStaticMethod<void>(
            kBleMidiClass,
            "disconnect",
            "()V");
    m_connected = false;
}

void BleMidiController::onMidiDataReceived(const QByteArray& data) {
    if (!s_pInstance || data.isEmpty())
        return;

    // Java already strips BLE MIDI protocol headers — we receive raw MIDI bytes.
    // Pass them directly to Mixxx's MIDI subsystem.
    kLogger.info() << "BLE MIDI received" << data.size() << "bytes";
    s_pInstance->receive(data, mixxx::Duration());
}

bool BleMidiController::poll() {
    // BLE MIDI uses callbacks, not polling
    return false;
}

void BleMidiController::sendShortMsg(unsigned char status,
        unsigned char byte1,
        unsigned char byte2) {
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

    QJniObject jAddress = QJniObject::fromString(m_deviceAddress);
    QJniObject jData = QJniObject::fromString(QString(packet.toHex()));
    QJniObject::callStaticMethod<void>(
            kBleScannerClass,
            "writeMidiData",
            "(Ljava/lang/String;Ljava/lang/String;)V",
            jAddress.object<jstring>(),
            jData.object<jstring>());
}

bool BleMidiController::sendBytes(const QByteArray& data) {
    QByteArray packet;
    qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    unsigned char tsHigh = 0x80 | ((nowMs >> 7) & 0x3F);
    unsigned char tsLow = 0x80 | (nowMs & 0x7F);
    packet.append(tsHigh);
    packet.append(tsLow);
    packet.append(data);

    QJniObject jAddress = QJniObject::fromString(m_deviceAddress);
    QJniObject jData = QJniObject::fromString(QString(packet.toHex()));
    QJniObject::callStaticMethod<void>(
            kBleScannerClass,
            "writeMidiData",
            "(Ljava/lang/String;Ljava/lang/String;)V",
            jAddress.object<jstring>(),
            jData.object<jstring>());
    return true;
}

#endif // Q_OS_ANDROID
