#include "controllers/hid/hidcontroller.h"

#include <cstring>
#include <cwchar>

#include "controllers/controllerdebug.h"
#include "controllers/defs_controllers.h"
#include "moc_hidcontroller.cpp"
#include "util/path.h" // for PATH_MAX on Windows
#include "util/time.h"
#include "util/trace.h"

namespace {
constexpr size_t kHidPathBufferSize = PATH_MAX + 1;
constexpr size_t kHidSerialMaxLength = 512;
constexpr size_t kHidSerialBufferSize = kHidSerialMaxLength + 1;
} // namespace

HidController::HidController(const hid_device_info& deviceInfo, UserSettingsPointer pConfig)
        : Controller(pConfig),
          m_pHidDevice(nullptr),
          m_iPollingBufferIndex(0) {
    // Copy required variables from deviceInfo, which will be freed after
    // this class is initialized by caller.
    hid_vendor_id = deviceInfo.vendor_id;
    hid_product_id = deviceInfo.product_id;
    hid_interface_number = deviceInfo.interface_number;
    if (hid_interface_number == -1) {
        // OS/X and windows don't use interface numbers, but usage_page/usage
        hid_usage_page = deviceInfo.usage_page;
        hid_usage = deviceInfo.usage;
    } else {
        // Linux hidapi does not set value for usage_page or usage and uses
        // interface number to identify subdevices
        hid_usage_page = 0;
        hid_usage = 0;
    }

    // Don't trust path to be null terminated.
    hid_path = new char[kHidPathBufferSize];
    std::strncpy(hid_path, deviceInfo.path, PATH_MAX);
    hid_path[PATH_MAX] = '\0';

    hid_serial_raw = nullptr;
    if (deviceInfo.serial_number != nullptr) {
        hid_serial_raw = new wchar_t[kHidSerialBufferSize];
        std::wcsncpy(hid_serial_raw, deviceInfo.serial_number, kHidSerialMaxLength);
        hid_serial_raw[kHidSerialMaxLength] = '\0';
    }

    hid_serial = safeDecodeWideString(deviceInfo.serial_number, 512);
    hid_manufacturer = safeDecodeWideString(deviceInfo.manufacturer_string, 512);
    hid_product = safeDecodeWideString(deviceInfo.product_string, 512);

    guessDeviceCategory();

    // Set the Unique Identifier to the serial_number
    m_sUID = hid_serial;

    //Note: We include the last 4 digits of the serial number and the
    // interface number to allow the user (and Mixxx!) to keep track of
    // which is which
    if (hid_interface_number < 0) {
        setDeviceName(
                QString("%1 %2").arg(hid_product, hid_serial.right(4)));
    } else {
        setDeviceName(QString("%1 %2_%3")
                              .arg(hid_product,
                                      hid_serial.right(4),
                                      QString::number(hid_interface_number)));
        m_sUID.append(QString::number(hid_interface_number));
    }

    // All HID devices are full-duplex
    setInputDevice(true);
    setOutputDevice(true);
}

HidController::~HidController() {
    if (isOpen()) {
        close();
    }
    delete [] hid_path;
    delete [] hid_serial_raw;
}

QString HidController::presetExtension() {
    return HID_PRESET_EXTENSION;
}

void HidController::visit(const MidiControllerPreset* preset) {
    Q_UNUSED(preset);
    // TODO(XXX): throw a hissy fit.
    qWarning() << "ERROR: Attempting to load a MidiControllerPreset to an HidController!";
}

void HidController::visit(const HidControllerPreset* preset) {
    m_preset = *preset;
    // Emit presetLoaded with a clone of the preset.
    emit presetLoaded(getPreset());
}

bool HidController::matchPreset(const PresetInfo& preset) {
    const QList<ProductInfo>& products = preset.getProducts();
    for (const auto& product : products) {
        if (matchProductInfo(product)) {
            return true;
        }
    }
    return false;
}

bool HidController::matchProductInfo(const ProductInfo& product) {
    int value;
    bool ok;
    // Product and vendor match is always required
    value = product.vendor_id.toInt(&ok,16);
    if (!ok || hid_vendor_id != value) {
        return false;
    }
    value = product.product_id.toInt(&ok,16);
    if (!ok || hid_product_id != value) {
        return false;
    }

    // Optionally check against interface_number / usage_page && usage
    if (hid_interface_number!=-1) {
        value = product.interface_number.toInt(&ok,16);
        if (!ok || hid_interface_number != value) {
            return false;
        }
    } else {
        value = product.usage_page.toInt(&ok,16);
        if (!ok || hid_usage_page != value) {
            return false;
        }

        value = product.usage.toInt(&ok,16);
        if (!ok || hid_usage != value) {
            return false;
        }
    }
    // Match found
    return true;
}

void HidController::guessDeviceCategory() {
    // This should be done somehow else, I know. But at least we get started with
    // the idea of mapping this information
    QString info;
    if (hid_interface_number==-1) {
        if (hid_usage_page==0x1) {
            switch (hid_usage) {
                case 0x2: info = tr("Generic HID Mouse"); break;
                case 0x4: info = tr("Generic HID Joystick"); break;
                case 0x5: info = tr("Generic HID Gamepad"); break;
                case 0x6: info = tr("Generic HID Keyboard"); break;
                case 0x8: info = tr("Generic HID Multiaxis Controller"); break;
                default: info = tr("Unknown HID Desktop Device") +
                    QStringLiteral(" 0x") + QString::number(hid_usage_page, 16) +
                    QStringLiteral("/0x") + QString::number(hid_usage, 16);
                    break;
            }
        } else if (hid_vendor_id==0x5ac) {
            // Apple laptop special HID devices
            if (hid_product_id==0x8242) {
                info = tr("HID Infrared Control");
            } else {
                info = tr("Unknown Apple HID Device") +
                    QStringLiteral(" 0x") + QString::number(hid_usage_page, 16) +
                    QStringLiteral("/0x") + QString::number(hid_usage, 16);
            }
        } else {
            // Fill in the usage page and usage fields for debugging info
            info = tr("HID Unknown Device") +
                QStringLiteral(" 0x") + QString::number(hid_usage_page, 16) +
                QStringLiteral("/0x") + QString::number(hid_usage, 16);
        }
    } else {
        // Guess linux device types somehow as well. Or maybe just fill in the
        // interface number?
        info = tr("HID Interface Number") +
            QStringLiteral(" 0x") + QString::number(hid_usage_page, 16) +
            QStringLiteral("/0x") + QString::number(hid_usage, 16);
    }
    setDeviceCategory(info);
}

int HidController::open() {
    if (isOpen()) {
        qDebug() << "HID device" << getName() << "already open";
        return -1;
    }

    // Open device by path
    controllerDebug("Opening HID device" << getName() << "by HID path" << hid_path);

    m_pHidDevice = hid_open_path(hid_path);

    // If that fails, try to open device with vendor/product/serial #
    if (m_pHidDevice == nullptr) {
        controllerDebug("Failed. Trying to open with make, model & serial no:"
                << hid_vendor_id << hid_product_id << hid_serial);
        m_pHidDevice = hid_open(hid_vendor_id, hid_product_id, hid_serial_raw);
    }

    // If it does fail, try without serial number WARNING: This will only open
    // one of multiple identical devices
    if (m_pHidDevice == nullptr) {
        qWarning() << "Unable to open specific HID device" << getName()
                   << "Trying now with just make and model."
                   << "(This may only open the first of multiple identical devices.)";
        m_pHidDevice = hid_open(hid_vendor_id, hid_product_id, nullptr);
    }

    // If that fails, we give up!
    if (m_pHidDevice == nullptr) {
        qWarning()  << "Unable to open HID device" << getName();
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

void HidController::send(QList<int> data, unsigned int length, unsigned int reportID) {
    Q_UNUSED(length);
    QByteArray temp;
    foreach (int datum, data) {
        temp.append(datum);
    }
    send(temp, reportID);
}

void HidController::send(const QByteArray& data) {
    send(data, 0);
}

void HidController::send(QByteArray data, unsigned int reportID) {
    // Append the Report ID to the beginning of data[] per the API..
    data.prepend(reportID);

    int result = hid_write(m_pHidDevice, (unsigned char*)data.constData(), data.size());
    if (result == -1) {
        if (ControllerDebug::enabled()) {
            qWarning() << "Unable to send data to" << getName()
                       << "serial #" << hid_serial << ":"
                       << safeDecodeWideString(hid_error(m_pHidDevice), 512);
        } else {
            qWarning() << "Unable to send data to" << getName() << ":"
                       << safeDecodeWideString(hid_error(m_pHidDevice), 512);
        }
    } else {
        controllerDebug(result << "bytes sent to" << getName()
                 << "serial #" << hid_serial
                 << "(including report ID of" << reportID << ")");
    }
}

//static
QString HidController::safeDecodeWideString(const wchar_t* pStr, size_t max_length) {
    if (pStr == nullptr) {
        return QString();
    }
    // find a terminating 0 or take all chars
    int size = 0;
    while ((size < (int)max_length) && (pStr[size] != 0)) {
        ++size;
    }
    // inlining QString::fromWCharArray()
    // We cannot use Qts wchar_t functions, since they may work or not
    // depending on the '/Zc:wchar_t-' build flag in the Qt configs
    // on Windows build
    if (sizeof(wchar_t) == sizeof(QChar)) {
        return QString::fromUtf16((const ushort *)pStr, size);
    } else {
        return QString::fromUcs4((uint *)pStr, size);
    }
}
