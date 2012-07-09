/**
  * @file hidcontroller.cpp
  * @author Sean M. Pappalardo	spappalardo@mixxx.org
  * @date Sun May 1 2011
  * @brief HID controller backend
  *
  */

#include <wchar.h>
#include <string.h>

#include "defs.h" // for PATH_MAX on Windows

#include "controllers/hid/hidcontroller.h"

HidReader::HidReader(hid_device* device)
        : QThread(),
          m_pHidDevice(device) {
}

HidReader::~HidReader() {
}

void HidReader::run() {
    m_stop = 0;
    unsigned char *data = new unsigned char[255];
    while (m_stop == 0) {
        // Blocked polling: The only problem with this is that we can't close
        // the device until the block is released, which means the controller
        // has to send more data
        //result = hid_read_timeout(m_pHidDevice, data, 255, -1);

        // This relieves that at the cost of higher CPU usage since we only
        // block for a short while (500ms)
        int result = hid_read_timeout(m_pHidDevice, data, 255, 500);
        if (result > 0) {
            //qDebug() << "Read" << result << "bytes, pointer:" << data;
            QByteArray outData(reinterpret_cast<char*>(data), result);
            emit(incomingData(outData));
        }
    }
    delete [] data;
}

QString safeDecodeWideString(wchar_t* pStr, size_t max_length) {
    if (pStr == NULL) {
        return QString();
    }
    // pStr is untrusted since it might be non-null terminated.
    wchar_t* tmp = new wchar_t[max_length+1];
    memset(tmp, 0, sizeof(tmp[0]) * sizeof(tmp));
    // wcsnlen is not available on all platforms, so just make a temporary
    // buffer
    wcsncpy(tmp, pStr, max_length);
    QString result = QString::fromWCharArray(tmp);
    delete [] tmp;
    return result;
}

HidController::HidController(const hid_device_info deviceInfo) {
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
    hid_path = new char[PATH_MAX+1];
    memset(hid_path, 0, sizeof(hid_path[0]) * sizeof(hid_path));
    strncpy(hid_path, deviceInfo.path, PATH_MAX);

    hid_serial_raw = NULL;
    if (deviceInfo.serial_number != NULL) {
        size_t serial_max_length = 512;
        hid_serial_raw = new wchar_t[serial_max_length+1];
        memset(hid_serial_raw, 0, sizeof(hid_serial_raw[0]) * sizeof(hid_serial_raw));
        wcsncpy(hid_serial_raw, deviceInfo.serial_number, serial_max_length);
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
            QString("%1 %2").arg(hid_product)
            .arg(hid_serial.right(4)));
    } else {
        setDeviceName(
            QString("%1 %2_%3").arg(hid_product)
            .arg(hid_serial.right(4))
            .arg(QString::number(hid_interface_number)));
        m_sUID.append(QString::number(hid_interface_number));
    }

    // All HID devices are full-duplex
    setInputDevice(true);
    setOutputDevice(true);
    m_pReader = NULL;
}

HidController::~HidController() {
    close();
    delete [] hid_path;
    delete [] hid_serial_raw;
}

void HidController::visit(const MidiControllerPreset* preset) {
    Q_UNUSED(preset);
    // TODO(XXX): throw a hissy fit.
    qDebug() << "ERROR: Attempting to load a MidiControllerPreset to an HidController!";
}

void HidController::visit(const HidControllerPreset* preset) {
    m_preset = *preset;
    // Emit presetLoaded with a clone of the preset.
    emit(presetLoaded(getPreset()));
}

bool HidController::savePreset(const QString fileName) const {
    HidControllerPresetFileHandler handler;
    return handler.save(m_preset, getName(), fileName);
}

bool HidController::matchPreset(const PresetInfo& preset) {
    const QList< QHash<QString,QString> > products = preset.getProducts();
    QHash <QString, QString> product;
    foreach (product, products) {
        if (matchProductInfo(product))
            return true;
    }
    return false;
}

bool HidController::matchProductInfo(QHash <QString,QString > info) {
    int value;
    bool ok;
    // Product and vendor match is always required
    value = info["vendor_id"].toInt(&ok,16);
    if (!ok || hid_vendor_id!=value) return false;
    value = info["product_id"].toInt(&ok,16);
    if (!ok || hid_product_id!=value) return false;

    // Optionally check against interface_number / usage_page && usage
    if (hid_interface_number!=-1) {
        value = info["interface_number"].toInt(&ok,16);
        if (!ok || hid_interface_number!=value) return false;
    } else {
        value = info["usage_page"].toInt(&ok,16);
        if (!ok || hid_usage_page!=value) return false;

        value = info["usage"].toInt(&ok,16);
        if (!ok || hid_usage!=value) return false;
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
                        QString().sprintf(" 0x%0x/0x%0x", hid_usage_page, hid_usage);
                    break;
            }
        } else if (hid_vendor_id==0x5ac) {
            // Apple laptop special HID devices
            if (hid_product_id==0x8242) {
                info = tr("HID Infrared Control");
            } else {
                info = tr("Unknown Apple HID Device") + QString().sprintf(
                    " 0x%0x/0x%0x",hid_usage_page,hid_usage);
            }
        } else {
            // Fill in the usage page and usage fields for debugging info
            info = tr("HID Unknown Device") + QString().sprintf(
                " 0x%0x/0x%0x", hid_usage_page, hid_usage);
        }
    } else {
        // Guess linux device types somehow as well. Or maybe just fill in the
        // interface number?
        info = tr("HID Interface Number") + QString().sprintf(
            " 0x%0x", hid_interface_number);
    }
    setDeviceCategory(info);
}

int HidController::open() {
    if (isOpen()) {
        qDebug() << "HID device" << getName() << "already open";
        return -1;
    }

    // Open device by path
    if (debugging()) {
        qDebug() << "Opening HID device"
                 << getName() << "by HID path" << hid_path;
    }
    m_pHidDevice = hid_open_path(hid_path);

    // If that fails, try to open device with vendor/product/serial #
    if (m_pHidDevice == NULL) {
        if (debugging())
            qDebug() << "Failed. Trying to open with make, model & serial no:"
                << hid_vendor_id << hid_product_id << hid_serial;
        m_pHidDevice = hid_open(hid_vendor_id, hid_product_id, hid_serial_raw);
    }

    // If it does fail, try without serial number WARNING: This will only open
    // one of multiple identical devices
    if (m_pHidDevice == NULL) {
        qWarning() << "Unable to open specific HID device" << getName()
                   << "Trying now with just make and model."
                   << "(This may only open the first of multiple identical devices.)";
        m_pHidDevice = hid_open(hid_vendor_id, hid_product_id, NULL);
    }

    // If that fails, we give up!
    if (m_pHidDevice == NULL) {
        qWarning()  << "Unable to open HID device" << getName();
        return -1;
    }

    setOpen(true);
    startEngine();

    if (m_pReader != NULL) {
        qWarning() << "HidReader already present for" << getName();
    } else {
        m_pReader = new HidReader(m_pHidDevice);
        m_pReader->setObjectName(QString("HidReader %1").arg(getName()));

        connect(m_pReader, SIGNAL(incomingData(QByteArray)),
                this, SLOT(receive(QByteArray)));

        // Controller input needs to be prioritized since it can affect the
        // audio directly, like when scratching
        m_pReader->start(QThread::HighPriority);
    }

    return 0;
}

int HidController::close() {
    if (!isOpen()) {
        qDebug() << "HID device" << getName() << "already closed";
        return -1;
    }

    qDebug() << "Shutting down HID device" << getName();

    // Stop the reading thread
    if (m_pReader == NULL) {
        qWarning() << "HidReader not present for" << getName()
                   << "yet the device is open!";
    } else {
        disconnect(m_pReader, SIGNAL(incomingData(QByteArray)),
                   this, SLOT(receive(QByteArray)));
        m_pReader->stop();
        hid_set_nonblocking(m_pHidDevice, 1);   // Quit blocking
        if (debugging()) qDebug() << "  Waiting on reader to finish";
        m_pReader->wait();
        delete m_pReader;
        m_pReader = NULL;
    }

    // Stop controller engine here to ensure it's done before the device is closed
    //  incase it has any final parting messages
    stopEngine();

    // Close device
    if (debugging()) {
        qDebug() << "  Closing device";
    }
    hid_close(m_pHidDevice);
    setOpen(false);
    return 0;
}

void HidController::send(QList<int> data, unsigned int length, unsigned int reportID) {
    Q_UNUSED(length);
    QByteArray temp;
    foreach (int datum, data) {
        temp.append(datum);
    }
    send(temp, reportID);
}

void HidController::send(QByteArray data) {
    send(data, 0);
}

void HidController::send(QByteArray data, unsigned int reportID) {
    // Append the Report ID to the beginning of data[] per the API..
    data.prepend(reportID);

    int result = hid_write(m_pHidDevice, (unsigned char*)data.constData(), data.size());
    if (result == -1) {
        if (debugging()) {
            qWarning() << "Unable to send data to" << getName()
                       << "serial #" << hid_serial << ":"
                       << QString::fromWCharArray(hid_error(m_pHidDevice));
        } else {
            qWarning() << "Unable to send data to" << getName() << ":"
                       << QString::fromWCharArray(hid_error(m_pHidDevice));
        }
    } else if (debugging()) {
        qDebug() << result << "bytes sent to" << getName()
                 << "serial #" << hid_serial
                 << "(including report ID of" << reportID << ")";
    }
}
