#include "controllers/hid/hidcontroller.h"

#include <hidapi.h>

#include "controllers/defs_controllers.h"
#include "moc_hidcontroller.cpp"
#include "util/string.h"

class LegacyControllerMapping;

namespace {
constexpr size_t kMaxHidErrorMessageSize = 512;
} // namespace

HidController::HidController(
        mixxx::hid::DeviceInfo&& deviceInfo)
        : Controller(deviceInfo.formatName()),
          m_deviceInfo(std::move(deviceInfo)) {
    // We assume, that all HID controllers are full-duplex,
    // but a HID can also be input only (e.g. a USB HID mouse)
    setInputDevice(true);
    setOutputDevice(true);
}

HidController::~HidController() {
    if (isOpen()) {
        close();
    }
}

QString HidController::mappingExtension() {
    return HID_MAPPING_EXTENSION;
}

void HidController::setMapping(std::shared_ptr<LegacyControllerMapping> pMapping) {
    m_pMapping = downcastAndClone<LegacyHidControllerMapping>(pMapping.get());
}

std::shared_ptr<LegacyControllerMapping> HidController::cloneMapping() {
    if (!m_pMapping) {
        return nullptr;
    }
    return std::make_shared<LegacyHidControllerMapping>(*m_pMapping);
}

bool HidController::matchMapping(const MappingInfo& mapping) {
    const QList<ProductInfo>& products = mapping.getProducts();
    for (const auto& product : products) {
        if (m_deviceInfo.matchProductInfo(product)) {
            return true;
        }
    }
    return false;
}

int HidController::open() {
    if (isOpen()) {
        qDebug() << "HID device" << getName() << "already open";
        return -1;
    }

    VERIFY_OR_DEBUG_ASSERT(!m_pHidIoThread) {
        qWarning() << "HidIoThread already present for" << getName();
        return -1;
    }

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

    // Set hid controller to non-blocking
    if (hid_set_nonblocking(pHidDevice, 1) != 0) {
        qCWarning(m_logBase) << "Unable to set HID device " << getName() << " to non-blocking";
        hid_close(pHidDevice);
        return -1;
    }

    setOpen(true);

    m_pHidIoThread = std::make_unique<HidIoThread>(pHidDevice, m_deviceInfo);
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

    return 0;
}

int HidController::close() {
    if (!isOpen()) {
        qCWarning(m_logBase) << "HID device" << getName() << "already closed";
        return -1;
    }

    qCInfo(m_logBase) << "Shutting down HID device" << getName();

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
