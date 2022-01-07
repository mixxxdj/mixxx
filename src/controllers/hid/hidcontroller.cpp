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
          m_deviceInfo(std::move(deviceInfo)),
          m_pHidDevice(nullptr) {
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
        
    if (m_pHidIoThread) {
        qWarning() << "HidIoThread already present for" << getName();
        return -1;
    }

    // Open device by path
    qCInfo(m_logBase) << "Opening HID device" << getName() << "by HID path"
                      << m_deviceInfo.pathRaw();

    m_pHidDevice = hid_open_path(m_deviceInfo.pathRaw());

    // If that fails, try to open device with vendor/product/serial #
    if (!m_pHidDevice) {
        qCWarning(m_logBase) << "Failed. Trying to open with make, model & serial no:"
                             << m_deviceInfo.vendorId() << m_deviceInfo.productId()
                             << m_deviceInfo.serialNumber();
        m_pHidDevice = hid_open(
                m_deviceInfo.vendorId(),
                m_deviceInfo.productId(),
                m_deviceInfo.serialNumberRaw());
    }

    // If it does fail, try without serial number WARNING: This will only open
    // one of multiple identical devices
    if (!m_pHidDevice) {
        qCWarning(m_logBase) << "Unable to open specific HID device" << getName()
                             << "Trying now with just make and model."
                             << "(This may only open the first of multiple identical devices.)";
        m_pHidDevice = hid_open(m_deviceInfo.vendorId(),
                m_deviceInfo.productId(),
                nullptr);
    }

    // If that fails, we give up!
    if (!m_pHidDevice) {
        qCWarning(m_logBase) << "Unable to open HID device" << getName();
        return -1;
    }

    // Set hid controller to non-blocking
    if (hid_set_nonblocking(m_pHidDevice, 1) != 0) {
        qCWarning(m_logBase) << "Unable to set HID device " << getName() << " to non-blocking";
        hid_close(m_pHidDevice);
        return -1;
    }

    setOpen(true);
    startEngine();

    m_pHidIoThread = std::make_unique<HidIoThread>(m_pHidDevice, std::move(m_deviceInfo));
    m_pHidIoThread->setObjectName(QString("HidIoThread %1").arg(getName()));

    connect(m_pHidIoThread.get(),
            &HidIoThread::receive,
            this,
            &HidController::receive,
            Qt::QueuedConnection);

    connect(this,
            &HidController::sendOutputReport,
            m_pHidIoThread.get(),
            &HidIoThread::sendOutputReport,
            Qt::QueuedConnection);

    // Controller input needs to be prioritized since it can affect the
    // audio directly, like when scratching
    m_pHidIoThread->start(QThread::HighPriority);
    m_pHidIoThread->startPollTimer();

    return 0;
}

int HidController::close() {
    if (!isOpen()) {
        qCWarning(m_logBase) << "HID device" << getName() << "already closed";
        return -1;
    }

    qCInfo(m_logBase) << "Shutting down HID device" << getName();

    // Stop the InputReport polling
    if (!m_pHidIoThread) {
        qWarning() << "HidIoThread not present for" << getName()
                   << "yet the device is open!";
    } else {
        disconnect(m_pHidIoThread.get());

        m_pHidIoThread->stopPollTimer();
        m_pHidIoThread->quit();
        qDebug() << "Waiting on IO thread to finish";
        m_pHidIoThread->wait();
    }

    // Stop controller engine here to ensure it's done before the device is closed
    // in case it has any final parting messages
    stopEngine();


    // Close device
    qCInfo(m_logBase) << "Closing device";
    // hid_close is not thread safe
    // All communication to this hid_device must be completed, before hid_close is called.
    hid_close(m_pHidDevice);
    setOpen(false);
    return 0;
}

void HidController::sendReport(QList<int> data, unsigned int length, unsigned int reportID) {
    Q_UNUSED(length);
    QByteArray temp;
    foreach (int datum, data) {
        temp.append(datum);
    }
    emit sendOutputReport(temp, reportID);
}

void HidController::sendBytes(const QByteArray& data) {
    emit sendOutputReport(data, 0);
}

ControllerJSProxy* HidController::jsProxy() {
    return new HidControllerJSProxy(this);
}
