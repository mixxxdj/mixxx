#include "controllers/bulk/bulkcontroller.h"

#include <libusb.h>

#include "controllers/bulk/bulksupported.h"
#include "controllers/controllerdebug.h"
#include "controllers/defs_controllers.h"
#include "moc_bulkcontroller.cpp"
#include "util/compatibility.h"
#include "util/time.h"
#include "util/trace.h"

BulkReader::BulkReader(libusb_device_handle *handle, unsigned char in_epaddr)
        : QThread(),
          m_phandle(handle),
          m_stop(0),
          m_in_epaddr(in_epaddr) {
}

BulkReader::~BulkReader() {
}

void BulkReader::stop() {
    m_stop = 1;
}

void BulkReader::run() {
    m_stop = 0;
    unsigned char data[255];

    while (atomicLoadAcquire(m_stop) == 0) {
        // Blocked polling: The only problem with this is that we can't close
        // the device until the block is released, which means the controller
        // has to send more data
        //result = hid_read_timeout(m_pHidDevice, data, 255, -1);

        // This relieves that at the cost of higher CPU usage since we only
        // block for a short while (500ms)
        int transferred;
        int result;

        result = libusb_bulk_transfer(m_phandle,
                                      m_in_epaddr,
                                      data, sizeof(data),
                                      &transferred, 500);
        Trace timeout("BulkReader timeout");
        if (result >= 0) {
            Trace process("BulkReader process packet");
            //qDebug() << "Read" << result << "bytes, pointer:" << data;
            QByteArray outData((char*)data, transferred);
            emit incomingData(outData, mixxx::Time::elapsed());
        }
    }
    qDebug() << "Stopped Reader";
}

static QString get_string(libusb_device_handle *handle, u_int8_t id) {
    unsigned char buf[128] = { 0 };

    if (id) {
        libusb_get_string_descriptor_ascii(handle, id, buf, sizeof(buf));
    }

    return QString::fromLatin1((char*)buf);
}

BulkController::BulkController(
        libusb_context* context,
        libusb_device_handle* handle,
        struct libusb_device_descriptor* desc)
        : m_context(context),
          m_phandle(handle),
          in_epaddr(0),
          out_epaddr(0) {
    vendor_id = desc->idVendor;
    product_id = desc->idProduct;

    manufacturer = get_string(handle, desc->iManufacturer);
    product = get_string(handle, desc->iProduct);
    m_sUID = get_string(handle, desc->iSerialNumber);

    setDeviceCategory(tr("USB Controller"));

    setDeviceName(QString("%1 %2").arg(product, m_sUID));

    setInputDevice(true);
    setOutputDevice(true);
    m_pReader = nullptr;
}

BulkController::~BulkController() {
    if (isOpen()) {
        close();
    }
}

QString BulkController::presetExtension() {
    return BULK_PRESET_EXTENSION;
}

void BulkController::visit(const MidiControllerPreset* preset) {
    Q_UNUSED(preset);
    // TODO(XXX): throw a hissy fit.
    qWarning() << "ERROR: Attempting to load a MidiControllerPreset to an HidController!";
}

void BulkController::visit(const HidControllerPreset* preset) {
    m_preset = *preset;
    // Emit presetLoaded with a clone of the preset.
    emit presetLoaded(getPreset());
}

bool BulkController::matchPreset(const PresetInfo& preset) {
    const QList<ProductInfo>& products = preset.getProducts();
    for (const auto& product : products) {
        if (matchProductInfo(product)) {
            return true;
        }
    }
    return false;
}

bool BulkController::matchProductInfo(const ProductInfo& product) {
    int value;
    bool ok;
    // Product and vendor match is always required
    value = product.vendor_id.toInt(&ok, 16);
    if (!ok || vendor_id != value) {
        return false;
    }
    value = product.product_id.toInt(&ok, 16);
    if (!ok || product_id != value) {
        return false;
    }

    // Match found
    return true;
}

int BulkController::open() {
    if (isOpen()) {
        qDebug() << "USB Bulk device" << getName() << "already open";
        return -1;
    }

    /* Look up endpoint addresses in supported database */
    int i;
    for (i = 0; bulk_supported[i].vendor_id; ++i) {
        if ((bulk_supported[i].vendor_id == vendor_id) &&
            (bulk_supported[i].product_id == product_id)) {
            in_epaddr = bulk_supported[i].in_epaddr;
            out_epaddr = bulk_supported[i].out_epaddr;
            break;
        }
    }

    if (bulk_supported[i].vendor_id == 0) {
        qWarning() << "USB Bulk device" << getName() << "unsupported";
        return -1;
    }

    // XXX: we should enumerate devices and match vendor, product, and serial
    if (m_phandle == nullptr) {
        m_phandle = libusb_open_device_with_vid_pid(
            m_context, vendor_id, product_id);
    }

    if (m_phandle == nullptr) {
        qWarning()  << "Unable to open USB Bulk device" << getName();
        return -1;
    }

    setOpen(true);
    startEngine();

    if (m_pReader != nullptr) {
        qWarning() << "BulkReader already present for" << getName();
    } else {
        m_pReader = new BulkReader(m_phandle, in_epaddr);
        m_pReader->setObjectName(QString("BulkReader %1").arg(getName()));

        connect(m_pReader, &BulkReader::incomingData, this, &BulkController::receive);

        // Controller input needs to be prioritized since it can affect the
        // audio directly, like when scratching
        m_pReader->start(QThread::HighPriority);
    }

    return 0;
}

int BulkController::close() {
    if (!isOpen()) {
        qDebug() << " device" << getName() << "already closed";
        return -1;
    }

    qDebug() << "Shutting down USB Bulk device" << getName();

    // Stop the reading thread
    if (m_pReader == nullptr) {
        qWarning() << "BulkReader not present for" << getName()
                   << "yet the device is open!";
    } else {
        disconnect(m_pReader, &BulkReader::incomingData, this, &BulkController::receive);
        m_pReader->stop();
        controllerDebug("  Waiting on reader to finish");
        m_pReader->wait();
        delete m_pReader;
        m_pReader = nullptr;
    }

    // Stop controller engine here to ensure it's done before the device is
    // closed in case it has any final parting messages
    stopEngine();

    // Close device
    controllerDebug("  Closing device");
    libusb_close(m_phandle);
    m_phandle = nullptr;
    setOpen(false);
    return 0;
}

void BulkController::send(const QList<int>& data, unsigned int length) {
    Q_UNUSED(length);
    QByteArray temp;

    foreach (int datum, data) {
        temp.append(datum);
    }
    sendBytes(temp);
}

void BulkController::sendBytes(const QByteArray& data) {
    int ret;
    int transferred;

    // XXX: don't get drunk again.
    ret = libusb_bulk_transfer(m_phandle, out_epaddr,
                               (unsigned char *)data.constData(), data.size(),
                               &transferred, 0);
    if (ret < 0) {
        qWarning() << "Unable to send data to" << getName()
                   << "serial #" << m_sUID;
    } else {
        controllerDebug(ret << "bytes sent to" << getName()
                 << "serial #" << m_sUID);
    }
}
