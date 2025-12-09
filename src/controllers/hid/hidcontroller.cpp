#include "controllers/hid/hidcontroller.h"

#ifdef __ANDROID__
#include <hidapi_libusb.h>
#include <libusb.h>

#include "controllers/android.h"
#else
#include <hidapi.h>
#endif
#include <QtConcurrent/QtConcurrentRun>

#include "controllers/defs_controllers.h"
#include "moc_hidcontroller.cpp"
#include "util/string.h"

class LegacyControllerMapping;

#ifndef __ANDROID__
namespace {
constexpr size_t kMaxHidErrorMessageSize = 512;
} // namespace
#endif

HidController::HidController(
        mixxx::hid::DeviceInfo&& deviceInfo)
        : Controller(deviceInfo.formatName()),
          m_deviceInfo(std::move(deviceInfo)),
          m_deviceUsesReportIds(std::nullopt) {
    // We assume, that all HID controllers are full-duplex,
    // but a HID can also be input only (e.g. a USB HID mouse)
    setInputDevice(true);
    setOutputDevice(true);

    // Start background fetch of report descriptor to avoid blocking controller
    // enumeration at startup.
    fetchReportDescriptorInBackground();
}

HidController::~HidController() {
    // Wait for background report descriptor fetching thread to finish if running.
    if (m_reportDescriptorFuture.isRunning()) {
        m_reportDescriptorFuture.waitForFinished();
    }

    if (isOpen()) {
        close();
    }
}

QString HidController::mappingExtension() {
    return HID_MAPPING_EXTENSION;
}

void HidController::setMapping(std::shared_ptr<LegacyControllerMapping> pMapping) {
    m_pMutableMapping = pMapping;
    m_pMapping = downcastAndClone<LegacyHidControllerMapping>(pMapping.get());
}

QList<LegacyControllerMapping::ScriptFileInfo> HidController::getMappingScriptFiles() {
    if (!m_pMapping) {
        return {};
    }
    return m_pMapping->getScriptFiles();
}

QList<std::shared_ptr<AbstractLegacyControllerSetting>> HidController::getMappingSettings() {
    if (!m_pMapping) {
        return {};
    }
    return m_pMapping->getSettings();
}

#ifdef MIXXX_USE_QML
QList<LegacyControllerMapping::QMLModuleInfo> HidController::getMappingModules() {
    if (!m_pMapping) {
        return {};
    }
    return m_pMapping->getModules();
}

QList<LegacyControllerMapping::ScreenInfo> HidController::getMappingInfoScreens() {
    if (!m_pMapping) {
        return {};
    }
    return m_pMapping->getInfoScreens();
}
#endif

bool HidController::matchMapping(const MappingInfo& mapping) {
    const QList<ProductInfo>& products = mapping.getProducts();
    for (const auto& product : products) {
        if (m_deviceInfo.matchProductInfo(product)) {
            return true;
        }
    }
    return false;
}

int HidController::open(const QString& resourcePath) {
    if (isOpen()) {
        qDebug() << "HID device" << getName() << "already open";
        return -1;
    }

    // Acquire a persistent lock protecting m_reportDescriptor and
    // m_deviceUsesReportIds. The lock is intentionally kept for the entire time
    // the device is considered "open":
    // - It is acquired here in open() and moved into `m_reportDescriptorLock`
    // so the `unique_lock` stays alive.
    // - It is released only when `close()` calls
    // `m_reportDescriptorLock.reset()` (or when the `HidController` is
    // destroyed
    //   and the stored `unique_lock` is destructed).
    // Note: `fetchReportDescriptorInBackground()` uses `try_to_lock` on the
    // same mutex and will skip fetching while the persistent lock is held.
    std::unique_lock<std::mutex> lock(m_reportDescriptorMutex);
    m_reportDescriptorLock.emplace(std::move(lock));

    VERIFY_OR_DEBUG_ASSERT(!m_pHidIoThread) {
        qWarning() << "HidIoThread already present for" << getName();
        return -1;
    }

#ifdef __ANDROID__
    QJniObject usbDeviceConnection;

    QJniObject context = QNativeInterface::QAndroidApplication::context();
    QJniObject USB_SERVICE =
            QJniObject::getStaticObjectField(
                    "android/content/Context",
                    "USB_SERVICE",
                    "Ljava/lang/String;");
    auto usbManager = context.callObjectMethod("getSystemService",
            "(Ljava/lang/String;)Ljava/lang/Object;",
            USB_SERVICE.object());
    if (!usbManager.isValid()) {
        qDebug() << "usbManager invalid";
        return -1;
    }

    auto usbDevice = m_deviceInfo.androidUsbDevice();

    if (!usbManager.callMethod<jboolean>("hasPermission",
                "(Landroid/hardware/usb/UsbDevice;)Z",
                usbDevice)) {
        auto pendingIntent = mixxx::android::getIntent();
        usbManager.callMethod<void>("requestPermission",
                "(Landroid/hardware/usb/UsbDevice;Landroid/app/"
                "PendingIntent;)V",
                usbDevice,
                pendingIntent);
        // Wait for permission
        if (!mixxx::android::waitForPermission(usbDevice)) {
            qDebug() << "access to device wasn't granted";
            return -1;
        }
        m_deviceInfo.updateSerialNumber(
                usbDevice.callMethod<jstring>("getSerialNumber").toString());
    }
    usbDeviceConnection = usbManager.callMethod<jobject>("openDevice",
            "(Landroid/hardware/usb/UsbDevice;)Landroid/hardware/usb/"
            "UsbDeviceConnection;",
            usbDevice);

    if (!usbDeviceConnection.isValid()) {
        qDebug() << "Unable to open HID device";
        return -1;
    }

    auto fileDescriptor = static_cast<intptr_t>(
            usbDeviceConnection.callMethod<jint>("getFileDescriptor"));

    // Open device by file descriptor
    qCInfo(m_logBase) << "Opening HID device" << getName()
                      << "by file descriptor"
                      << fileDescriptor << "and interface"
                      << m_deviceInfo.getUsbInterfaceNumber();

    libusb_set_option(nullptr, LIBUSB_OPTION_NO_DEVICE_DISCOVERY);
    hid_device* pHidDevice = hid_libusb_wrap_sys_device(
            fileDescriptor, m_deviceInfo.getUsbInterfaceNumber().value());
#else
    // Open device by path
    qCInfo(m_logBase) << "Opening HID device" << getName() << "by HID path"
                      << m_deviceInfo.pathRaw();

    hid_device* pHidDevice = hid_open_path(m_deviceInfo.pathRaw());

    // If that fails, try to open device with vendor/product/serial #
    if (!pHidDevice) {
        qCWarning(m_logBase) << QStringLiteral(
                "Unable to open specific HID device %1 using its path %2: %3")
                                        .arg(getName(),
                                                m_deviceInfo.pathRaw(),
                                                mixxx::convertWCStringToQString(
                                                        hid_error(nullptr),
                                                        kMaxHidErrorMessageSize));
        qCInfo(m_logBase) << QStringLiteral(
                "Trying to open HID device %1 using its vendor, product and "
                "serial no (0x%2, 0x%3 and %4)")
                                     .arg(getName(),
                                             QString::number(m_deviceInfo.getVendorId(), 16),
                                             QString::number(m_deviceInfo.getProductId(), 16),
                                             m_deviceInfo.getSerialNumber());
        pHidDevice = hid_open(
                m_deviceInfo.getVendorId(),
                m_deviceInfo.getProductId(),
                m_deviceInfo.serialNumberRaw());
    }

    // If it does fail, try without serial number WARNING: This will only open
    // one of multiple identical devices
    if (!pHidDevice) {
        qCWarning(m_logBase) << QStringLiteral(
                "Unable to open specific HID device %1 using its vendor, "
                "product and serial no (0x%2, 0x%3 and %4): %5")
                                        .arg(getName(),
                                                QString::number(m_deviceInfo.getVendorId(), 16),
                                                QString::number(m_deviceInfo.getProductId(), 16),
                                                m_deviceInfo.getSerialNumber(),
                                                mixxx::convertWCStringToQString(
                                                        hid_error(nullptr),
                                                        kMaxHidErrorMessageSize));
        qCInfo(m_logBase) << QStringLiteral(
                "Trying to open HID device %1 using its vendor and product "
                "only (0x%2 and 0x%3). This may only open the first of multiple "
                "identical devices.")
                                     .arg(getName(),
                                             QString::number(m_deviceInfo.getVendorId(), 16),
                                             QString::number(m_deviceInfo.getProductId(), 16));
        pHidDevice = hid_open(m_deviceInfo.getVendorId(),
                m_deviceInfo.getProductId(),
                nullptr);
    }

    // If that fails, we give up!
    if (!pHidDevice) {
        qCWarning(m_logBase) << QStringLiteral(
                "Unable to open specific HID device %1 using its vendor and "
                "product only (0x%2 and 0x%3): %4")
                                        .arg(getName(),
                                                QString::number(m_deviceInfo.getVendorId(), 16),
                                                QString::number(m_deviceInfo.getProductId(), 16),
                                                mixxx::convertWCStringToQString(
                                                        hid_error(nullptr),
                                                        kMaxHidErrorMessageSize));
        return -1;
    }
#endif

    // Set hid controller to non-blocking
    if (hid_set_nonblocking(pHidDevice, 1) != 0) {
        qCWarning(m_logBase) << "Unable to set HID device " << getName() << " to non-blocking";
        hid_close(pHidDevice);
        return -1;
    }

#ifndef Q_OS_ANDROID
    if (!m_reportDescriptor || !m_deviceUsesReportIds.has_value()) {
        // If this is reached, the Report Descriptor wasn't fetched successful before
        // Try it now, as we have a live device handle
        const std::vector<uint8_t>& rawReportDescriptor =
                m_deviceInfo.fetchRawReportDescriptor(pHidDevice);

        if (!rawReportDescriptor.empty()) {
            m_reportDescriptor =
                    std::make_shared<hid::reportDescriptor::HidReportDescriptor>(
                            rawReportDescriptor);
            m_reportDescriptor->parse();
            m_deviceUsesReportIds = m_reportDescriptor->isDeviceWithReportIds();
        } else {
            m_reportDescriptor.reset();
            m_deviceUsesReportIds = std::nullopt;
        }
    }

#endif
    m_pHidIoThread = std::make_unique<HidIoThread>(pHidDevice, m_deviceInfo, m_deviceUsesReportIds);
#ifdef Q_OS_ANDROID
    m_pHidIoThread->setDeviceConnection(std::move(usbDeviceConnection));
#endif
    m_pHidIoThread->setObjectName(QStringLiteral("HidIoThread ") + getName());

    connect(m_pHidIoThread.get(),
            &HidIoThread::receive,
            this,
            &HidController::receive,
            Qt::QueuedConnection);

    // Controller input needs to be prioritized since it can affect the
    // audio directly, like when scratching
    // The effect of the priority parameter is dependent on the operating system's scheduling policy.
    // In particular, the priority will be ignored on systems that do not support thread priorities (as Linux).
    m_pHidIoThread->start(QThread::HighPriority);

    VERIFY_OR_DEBUG_ASSERT(m_pHidIoThread->testAndSetThreadState(
            HidIoThreadState::Initialized, HidIoThreadState::OutputActive)) {
        qWarning() << "HidIoThread wasn't in expected Initialized state";
    }

    // This executes the init function of the JavaScript mapping
    startEngine();

    VERIFY_OR_DEBUG_ASSERT(m_pHidIoThread->testAndSetThreadState(
            HidIoThreadState::OutputActive,
            HidIoThreadState::InputOutputActive)) {
        qWarning() << "HidIoThread wasn't in expected OutputActive state";
    }

    applyMapping(resourcePath);
    setOpen(true);
    return 0;
}

int HidController::close() {
    if (!isOpen()) {
        qCWarning(m_logBase) << "HID device" << getName() << "already closed";
        return -1;
    }

    qCInfo(m_logBase) << "Shutting down HID device" << getName();

    // Release persistent report descriptor lock acquired in open(), if any.
    // The requested behaviour is to keep the mutex locked while the device is open
    // and only release it when close() runs.
    if (m_reportDescriptorLock.has_value()) {
        m_reportDescriptorLock.reset();
    }

    // Stop the InputReport polling, but allow sending OutputReports in JavaScript mapping shutdown procedure
    VERIFY_OR_DEBUG_ASSERT(m_pHidIoThread) {
        qWarning() << "HidIoThread not present for" << getName()
                   << "while closing the device !";
    }
    else {
        VERIFY_OR_DEBUG_ASSERT(m_pHidIoThread->testAndSetThreadState(
                HidIoThreadState::InputOutputActive,
                HidIoThreadState::OutputActive)) {
            qWarning() << "HidIoThread wasn't in expected InputOutputActive state";
        }
    }

    // Stop controller engine here to ensure it's done before the device is closed
    // in case it has any final parting messages
    // This executes the shutdown function of the JavaScript mapping
    stopEngine();

    if (m_pHidIoThread) {
        disconnect(m_pHidIoThread.get());

        // Request stop after sending the last cached OutputReport
        VERIFY_OR_DEBUG_ASSERT(m_pHidIoThread->testAndSetThreadState(
                HidIoThreadState::OutputActive,
                HidIoThreadState::StopWhenAllReportsSent)) {
            qWarning() << "HidIoThread wasn't in expected OutputActive state";
            m_pHidIoThread->setThreadState(HidIoThreadState::StopWhenAllReportsSent);
        }

        qCInfo(m_logBase) << "Waiting on HID IO thread to send the last remaining OutputReports";
        VERIFY_OR_DEBUG_ASSERT(m_pHidIoThread->waitUntilRunLoopIsStopped(50)) {
            qWarning() << "Sending the last remaining OutputReports "
                          "reached timeout!";

            qCInfo(m_logBase) << "Enforce stop of HID IO thread run loop!";
            m_pHidIoThread->setThreadState(HidIoThreadState::StopRequested);

            VERIFY_OR_DEBUG_ASSERT(m_pHidIoThread->waitUntilRunLoopIsStopped(100)) {
                qWarning() << "Stopping run loop reached timeout!";
            }
        }

        // After completion of all HID communication deconstruct m_pHidIoThread
        m_pHidIoThread.reset();
    }

    // Close device
    setOpen(false);
    qCInfo(m_logBase) << "Device closed";
    return 0;
}

/// This function is only for class compatibility with the (midi)controller
/// and will not do the same as for MIDI devices,
/// because sending of raw bytes is not a supported HIDAPI feature.
bool HidController::sendBytes(const QByteArray& data) {
    // Some HIDAPI backends will fail if the device uses ReportIDs (as practical all DJ controllers),
    // because 0 is no valid ReportID for these devices.
    m_pHidIoThread->updateCachedOutputReportData(0, data, false);
    return true;
}

ControllerJSProxy* HidController::jsProxy() {
    return new HidControllerJSProxy(this);
}

void HidController::fetchReportDescriptorInBackground() {
#ifndef Q_OS_ANDROID
    // Launch a concurrent task to open the device and fetch the report descriptor
    m_reportDescriptorFuture = QtConcurrent::run([this]() {
        // Try to acquire the mutex. If another thread already
        // locked the report descriptor mutex, skip the background fetch.
        std::unique_lock<std::mutex> lock(this->m_reportDescriptorMutex, std::try_to_lock);
        if (!lock.owns_lock()) {
            qCWarning(m_logBase) << "HID Report Descriptor structure is locked" << getName();
            return;
        }

        hid_device* pHidDevice = hid_open_path(this->m_deviceInfo.pathRaw());
        if (!pHidDevice) {
            pHidDevice = hid_open(this->m_deviceInfo.getVendorId(),
                    this->m_deviceInfo.getProductId(),
                    this->m_deviceInfo.serialNumberRaw());
        }
        if (!pHidDevice) {
            pHidDevice = hid_open(this->m_deviceInfo.getVendorId(),
                    this->m_deviceInfo.getProductId(),
                    nullptr);
        }
        if (!pHidDevice) {
            return;
        }
        hid_set_nonblocking(pHidDevice, 1);

        const std::vector<uint8_t>& rawReportDescriptor =
                this->m_deviceInfo.fetchRawReportDescriptor(pHidDevice);

        if (!rawReportDescriptor.empty()) {
            m_reportDescriptor = std::make_shared<
                    hid::reportDescriptor::HidReportDescriptor>(
                    rawReportDescriptor);
            m_reportDescriptor->parse();
            m_deviceUsesReportIds = m_reportDescriptor->isDeviceWithReportIds();
        }
        hid_close(pHidDevice);
    });
#else
    Q_UNUSED(m_reportDescriptorFuture);
#endif
}
