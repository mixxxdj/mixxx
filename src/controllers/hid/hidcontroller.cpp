#include "controllers/hid/hidcontroller.h"

#include <hidapi.h>

#include "controllers/defs_controllers.h"
#include "controllers/hid/legacyhidcontrollermappingfilehandler.h"
#include "moc_hidcontroller.cpp"
#include "util/string.h"
#include "util/time.h"
#include "util/trace.h"

HidController::HidController(
        mixxx::hid::DeviceInfo&& deviceInfo)
        : Controller(deviceInfo.formatName()),
          m_deviceInfo(std::move(deviceInfo)) {
    setDeviceCategory(mixxx::hid::DeviceCategory::guessFromDeviceInfo(m_deviceInfo));

    // All HID devices are full-duplex
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
    m_pMapping = downcastAndTakeOwnership<LegacyHidControllerMapping>(std::move(pMapping));
}

std::shared_ptr<LegacyControllerMapping> HidController::cloneMapping() {
    if (!m_pMapping) {
        return nullptr;
    }
    return m_pMapping->clone();
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
        qCWarning(m_logBase) << "Failed. Trying to open with make, model & serial no:"
                             << m_deviceInfo.vendorId() << m_deviceInfo.productId()
                             << m_deviceInfo.serialNumber();
        pHidDevice = hid_open(
                m_deviceInfo.vendorId(),
                m_deviceInfo.productId(),
                m_deviceInfo.serialNumberRaw());
    }

    // If it does fail, try without serial number WARNING: This will only open
    // one of multiple identical devices
    if (!pHidDevice) {
        qCWarning(m_logBase) << "Unable to open specific HID device" << getName()
                             << "Trying now with just make and model."
                             << "(This may only open the first of multiple identical devices.)";
        pHidDevice = hid_open(m_deviceInfo.vendorId(),
                m_deviceInfo.productId(),
                nullptr);
    }

    // If that fails, we give up!
    if (!pHidDevice) {
        qCWarning(m_logBase) << "Unable to open HID device" << getName();
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
    m_pHidIoThread->start(QThread::HighPriority);

    DEBUG_ASSERT(m_pHidIoThread->testAndSetThreadState(
            HidIoThreadState::Initialized, HidIoThreadState::OutputActive));

    // This executes the init function of the JavaScript mapping
    startEngine();

    DEBUG_ASSERT(m_pHidIoThread->testAndSetThreadState(
            HidIoThreadState::OutputActive,
            HidIoThreadState::InputOutputActive));

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
                   << "yet the device is open!";
    }
    else {
        m_pHidIoThread->testAndSetThreadState(
                HidIoThreadState::InputOutputActive,
                HidIoThreadState::OutputActive);
    }

    // Stop controller engine here to ensure it's done before the device is closed
    // in case it has any final parting messages
    // This executes the shutdown function of the JavaScript mapping
    stopEngine();

    if (m_pHidIoThread) {
        disconnect(m_pHidIoThread.get());

        // Request stop after sending the last latched OutputReport
        DEBUG_ASSERT(m_pHidIoThread->testAndSetThreadState(
                HidIoThreadState::OutputActive,
                HidIoThreadState::StopWhenAllReportsSent));

        qCInfo(m_logBase) << "Waiting on HID IO thread to finish";
        VERIFY_OR_DEBUG_ASSERT(m_pHidIoThread->testAndSetThreadState(
                HidIoThreadState::Stopped, HidIoThreadState::Stopped, 50)) {
            qWarning() << "Sending the last remaining OutputReports "
                          "reached timeout!";
            m_pHidIoThread->forceStopOfThreadRunLoop();
        }

        VERIFY_OR_DEBUG_ASSERT(m_pHidIoThread->testAndSetThreadState(
                HidIoThreadState::Stopped, HidIoThreadState::Stopped, 100)) {
            // Timeout: No response from thread
            qWarning() << "Stopping run loop reached timeout!";
        }

        // After completion of all HID communication deconstruct m_pHidIoThread
        m_pHidIoThread.reset();
    }

    // Close device
    qCInfo(m_logBase) << "Closing device";
    setOpen(false);
    return 0;
}

void HidController::sendReport(QList<int> data, unsigned int length, unsigned int reportID) {
    Q_UNUSED(length);
    QByteArray temp;
    temp.reserve(data.size());
    foreach (int datum, data) {
        temp.append(datum);
    }
    m_pHidIoThread->latchOutputReport(temp, reportID);
}

void HidController::sendBytes(const QByteArray& data) {
    m_pHidIoThread->latchOutputReport(data, 0);
}

ControllerJSProxy* HidController::jsProxy() {
    return new HidControllerJSProxy(this);
}
