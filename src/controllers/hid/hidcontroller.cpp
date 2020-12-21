#include "controllers/hid/hidcontroller.h"

#include <hidapi.h>

#include "controllers/controllerdebug.h"
#include "controllers/defs_controllers.h"
#include "controllers/hid/legacyhidcontrollermappingfilehandler.h"
#include "moc_hidcontroller.cpp"
#include "util/string.h"
#include "util/time.h"
#include "util/trace.h"

namespace {
constexpr int kReportIdSize = 1;
constexpr int kMaxHidErrorMessageSize = 512;
} // namespace

HidController::HidController(
        mixxx::hid::DeviceInfo&& deviceInfo)
        : m_deviceInfo(std::move(deviceInfo)),
          m_pHidDevice(nullptr),
          m_iPollingBufferIndex(0) {
    setDeviceCategory(mixxx::hid::DeviceCategory::guessFromDeviceInfo(m_deviceInfo));
    setDeviceName(m_deviceInfo.formatName());

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

void HidController::visit(const LegacyMidiControllerMapping* mapping) {
    Q_UNUSED(mapping);
    // TODO(XXX): throw a hissy fit.
    qWarning() << "ERROR: Attempting to load a LegacyMidiControllerMapping to an HidController!";
}

void HidController::visit(const LegacyHidControllerMapping* mapping) {
    m_mapping = *mapping;
    // Emit mappingLoaded with a clone of the mapping.
    emit mappingLoaded(getMapping());
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

    // Open device by path
    controllerDebug("Opening HID device" << getName() << "by HID path"
                                         << m_deviceInfo.pathRaw());

    m_pHidDevice = hid_open_path(m_deviceInfo.pathRaw());

    // If that fails, try to open device with vendor/product/serial #
    if (!m_pHidDevice) {
        controllerDebug("Failed. Trying to open with make, model & serial no:"
                << m_deviceInfo.vendorId() << m_deviceInfo.productId()
                << m_deviceInfo.serialNumber());
        m_pHidDevice = hid_open(
                m_deviceInfo.vendorId(),
                m_deviceInfo.productId(),
                m_deviceInfo.serialNumberRaw());
    }

    // If it does fail, try without serial number WARNING: This will only open
    // one of multiple identical devices
    if (!m_pHidDevice) {
        qWarning() << "Unable to open specific HID device" << getName()
                   << "Trying now with just make and model."
                   << "(This may only open the first of multiple identical devices.)";
        m_pHidDevice = hid_open(m_deviceInfo.vendorId(),
                m_deviceInfo.productId(),
                nullptr);
    }

    // If that fails, we give up!
    if (!m_pHidDevice) {
        qWarning() << "Unable to open HID device" << getName();
        return -1;
    }

    // Set hid controller to non-blocking
    if (hid_set_nonblocking(m_pHidDevice, 1) != 0) {
        qWarning() << "Unable to set HID device " << getName() << " to non-blocking";
        return -1;
    }

    // This isn't strictly necessary but is good practice.
    for (int i = 0; i < kNumBuffers; i++) {
        memset(m_pPollData[i], 0, kBufferSize);
    }
    m_iLastPollSize = 0;

    setOpen(true);
    startEngine();

    return 0;
}

int HidController::close() {
    if (!isOpen()) {
        qDebug() << "HID device" << getName() << "already closed";
        return -1;
    }

    qDebug() << "Shutting down HID device" << getName();

    // Stop controller engine here to ensure it's done before the device is closed
    //  in case it has any final parting messages
    stopEngine();

    // Close device
    controllerDebug("  Closing device");
    hid_close(m_pHidDevice);
    setOpen(false);
    return 0;
}

bool HidController::poll() {
    Trace hidRead("HidController poll");

    // This loop risks becoming a high priority endless loop in case processing
    // the mapping JS code takes longer than the controller polling rate.
    // This could stall other low priority tasks.
    // There is no safety net for this because it has not been demonstrated to be
    // a problem in practice.
    while (true) {
        // Cycle between buffers so the memcmp below does not require deep copying to another buffer.
        unsigned char* pPreviousBuffer = m_pPollData[m_iPollingBufferIndex];
        const int currentBufferIndex = (m_iPollingBufferIndex + 1) % kNumBuffers;
        unsigned char* pCurrentBuffer = m_pPollData[currentBufferIndex];

        int bytesRead = hid_read(m_pHidDevice, pCurrentBuffer, kBufferSize);
        if (bytesRead < 0) {
            // -1 is the only error value according to hidapi documentation.
            DEBUG_ASSERT(bytesRead == -1);
            return false;
        } else if (bytesRead == 0) {
            return true;
        }

        Trace process("HidController process packet");
        // Some controllers such as the Gemini GMX continuously send input packets even if it
        // is identical to the previous packet. If this loop processed all those redundant
        // packets, it would be a big performance problem to run JS code for every packet and
        // would be unnecessary.
        // This assumes that the redundant packets all use the same report ID. In practice we
        // have not encountered any controllers that send redundant packets with different report
        // IDs. If any such devices exist, this may be changed to use a separate buffer to store
        // the last packet for each report ID.
        if (bytesRead == m_iLastPollSize &&
                memcmp(pCurrentBuffer, pPreviousBuffer, bytesRead) == 0) {
            continue;
        }
        m_iLastPollSize = bytesRead;
        m_iPollingBufferIndex = currentBufferIndex;
        auto incomingData = QByteArray::fromRawData(
                reinterpret_cast<char*>(pCurrentBuffer), bytesRead);
        receive(incomingData, mixxx::Time::elapsed());
    }
}

bool HidController::isPolling() const {
    return isOpen();
}

void HidController::sendReport(QList<int> data, unsigned int length, unsigned int reportID) {
    Q_UNUSED(length);
    QByteArray temp;
    foreach (int datum, data) {
        temp.append(datum);
    }
    sendBytesReport(temp, reportID);
}

void HidController::sendBytes(const QByteArray& data) {
    sendBytesReport(data, 0);
}

void HidController::sendBytesReport(QByteArray data, unsigned int reportID) {
    // Append the Report ID to the beginning of data[] per the API..
    data.prepend(reportID);

    int result = hid_write(m_pHidDevice, (unsigned char*)data.constData(), data.size());
    if (result == -1) {
        if (ControllerDebug::isEnabled()) {
            qWarning() << "Unable to send data to" << getName()
                       << "serial #" << m_deviceInfo.serialNumber() << ":"
                       << mixxx::convertWCStringToQString(
                                  hid_error(m_pHidDevice),
                                  kMaxHidErrorMessageSize);
        } else {
            qWarning() << "Unable to send data to" << getName() << ":"
                       << mixxx::convertWCStringToQString(
                                  hid_error(m_pHidDevice),
                                  kMaxHidErrorMessageSize);
        }
    } else {
        controllerDebug(result << "bytes sent to" << getName()
                               << "serial #" << m_deviceInfo.serialNumber()
                               << "(including report ID of" << reportID << ")");
    }
}

void HidController::sendFeatureReport(
        const QList<int>& dataList, unsigned int reportID) {
    QByteArray dataArray;
    dataArray.reserve(kReportIdSize + dataList.size());

    // Append the Report ID to the beginning of dataArray[] per the API..
    dataArray.append(reportID);

    for (const int datum : dataList) {
        dataArray.append(datum);
    }

    int result = hid_send_feature_report(m_pHidDevice,
            reinterpret_cast<const unsigned char*>(dataArray.constData()),
            dataArray.size());
    if (result == -1) {
        qWarning() << "sendFeatureReport is unable to send data to"
                   << getName() << "serial #" << m_deviceInfo.serialNumber()
                   << ":"
                   << mixxx::convertWCStringToQString(
                              hid_error(m_pHidDevice),
                              kMaxHidErrorMessageSize);
    } else {
        controllerDebug(result << "bytes sent by sendFeatureReport to" << getName()
                               << "serial #" << m_deviceInfo.serialNumber()
                               << "(including report ID of" << reportID << ")");
    }
}

ControllerJSProxy* HidController::jsProxy() {
    return new HidControllerJSProxy(this);
}
