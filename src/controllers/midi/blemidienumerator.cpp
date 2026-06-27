#include "blemidienumerator.h"

#include "controllers/midi/blemidicontroller.h"
#include "util/logger.h"

#ifdef Q_OS_ANDROID
#include <QtCore/private/qandroidextras_p.h>

#include <QJniEnvironment>
#include <QJniObject>
#endif

namespace {

const mixxx::Logger kLogger("BleMidiEnumerator");

} // namespace

BleMidiEnumerator::BleMidiEnumerator(UserSettingsPointer pConfig)
        : m_pConfig(pConfig),
          m_pScanTimer(new QTimer(this)),
          m_scanning(false) {
    m_pScanTimer->setSingleShot(true);
    m_pScanTimer->setInterval(10000);
    connect(m_pScanTimer, &QTimer::timeout, this, &BleMidiEnumerator::slotOnScanTimeout);
}

BleMidiEnumerator::~BleMidiEnumerator() {
    m_devices.clear();
}

QList<Controller*> BleMidiEnumerator::queryDevices() {
    QMutexLocker locker(&m_mutex);
    return m_devices;
}

void BleMidiEnumerator::startScan() {
#ifdef Q_OS_ANDROID
    constexpr const char* kBleMidiServiceUuid = "03B80E5A-EDE8-4B33-A751-6CE34EC4C700";
    constexpr const char* kBleScannerClass = "org/mixxx/BleMidiScanner";

    if (m_scanning) {
        return;
    }
    m_scanning = true;

    QJniEnvironment env;
    if (!env.isValid()) {
        kLogger.warning() << "BLE scan: JNI environment not available";
        m_scanning = false;
        return;
    }

    QJniObject activity = QJniObject::callStaticObjectMethod(
            "org/qtproject/qt/android/QtNative",
            "activity",
            "()Landroid/app/Activity;");
    if (!activity.isValid()) {
        kLogger.warning() << "BLE scan: activity not available";
        m_scanning = false;
        return;
    }

    jboolean result = QJniObject::callStaticMethod<jboolean>(
            kBleScannerClass,
            "startScan",
            "(Landroid/content/Context;Ljava/lang/String;)Z",
            activity.object<jobject>(),
            QJniObject::fromString(QString(kBleMidiServiceUuid)).object<jstring>());

    if (!result) {
        kLogger.info() << "BLE scan: scanner returned no results or permissions denied";
        m_scanning = false;
        return;
    }

    m_pScanTimer->start();
    kLogger.info() << "BLE scan: started for MIDI service";
#else
    Q_UNUSED(m_pConfig);
    kLogger.info() << "BLE scan: not supported on this platform";
#endif
}

void BleMidiEnumerator::rescan() {
    startScan();
}

bool BleMidiEnumerator::isConnected() const {
#ifdef Q_OS_ANDROID
    constexpr const char* kBleScannerClass = "org/mixxx/BleMidiScanner";

    jboolean result = QJniObject::callStaticMethod<jboolean>(
            kBleScannerClass, "isConnected", "()Z");
    return result;
#else
    return false;
#endif
}

void BleMidiEnumerator::slotOnScanTimeout() {
#ifdef Q_OS_ANDROID
    constexpr const char* kBleScannerClass = "org/mixxx/BleMidiScanner";

    m_scanning = false;

    // Stop the BLE scan to save battery
    QJniObject::callStaticMethod<void>(
            kBleScannerClass,
            "stopScan",
            "()V");

    QJniObject devices = QJniObject::callStaticObjectMethod(
            kBleScannerClass,
            "getDiscoveredDevices",
            "()Ljava/util/List;");

    if (!devices.isValid()) {
        kLogger.warning() << "BLE scan: failed to get discovered devices";
        return;
    }

    // Iterate the Java List and create BleMidiController objects
    jint deviceCount = devices.callMethod<jint>("size", "()I");
    kLogger.info() << "BLE scan: completed, found"
                   << deviceCount << "BLE MIDI device(s)";

    QMutexLocker locker(&m_mutex);

    // Clear previous devices that are no longer found
    m_devices.clear();

    for (jint i = 0; i < deviceCount; i++) {
        QJniObject deviceMap = devices.callMethod<jobject>(
                "get", "(I)Ljava/lang/Object;", i);
        if (!deviceMap.isValid()) {
            continue;
        }

        // Extract device info from the Map
        QJniObject nameObj = deviceMap.callObjectMethod(
                "get",
                "(Ljava/lang/Object;)Ljava/lang/Object;",
                QJniObject::fromString(QString("name")).object<jstring>());
        QJniObject addressObj = deviceMap.callObjectMethod(
                "get",
                "(Ljava/lang/Object;)Ljava/lang/Object;",
                QJniObject::fromString(QString("address")).object<jstring>());

        QString deviceName = nameObj.isValid() ? nameObj.toString()
                                               : QString("BLE MIDI Device");
        QString deviceAddress = addressObj.isValid() ? addressObj.toString()
                                                     : QString();

        kLogger.info() << "BLE scan: found device" << deviceName
                       << "at" << deviceAddress;

        // Create a BleMidiController for this BLE MIDI device
        QString controllerName = deviceName + " (" + deviceAddress + ")";
        BleMidiController* pController =
                new BleMidiController(controllerName, deviceAddress);
        m_devices.push_back(pController);
    }

    if (deviceCount > 0) {
        kLogger.info() << "BLE scan: created" << m_devices.size()
                       << "BLE MIDI controller(s)";
    }
#endif
}

#include "moc_blemidienumerator.cpp"
