#include "controllers/bulk/bulkcontroller.h"

#include <libusb.h>

#include "controllers/bulk/bulksupported.h"
#include "controllers/defs_controllers.h"
#include "moc_bulkcontroller.cpp"
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

    while (m_stop.loadAcquire() == 0) {
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
            QByteArray byteArray(reinterpret_cast<char*>(data), transferred);
            emit incomingData(byteArray, mixxx::Time::elapsed());
        }
    }
    qDebug() << "Stopped Reader";
}

static QString get_string(libusb_device_handle* handle, uint8_t id) {
    unsigned char buf[128] = { 0 };

    if (id) {
        libusb_get_string_descriptor_ascii(handle, id, buf, sizeof(buf));
    }

    return QString::fromLatin1((char*)buf);
}

BulkController::BulkController(libusb_context* context,
        libusb_device_handle* handle,
        struct libusb_device_descriptor* desc)
        : Controller(QString("%1 %2").arg(
                  get_string(handle, desc->iProduct),
                  get_string(handle, desc->iSerialNumber))),
          m_context(context),
          m_phandle(handle),
          m_inEndpointAddr(0),
          m_outEndpointAddr(0) {
    m_vendorId = desc->idVendor;
    m_productId = desc->idProduct;

    m_manufacturer = get_string(handle, desc->iManufacturer);
    m_product = get_string(handle, desc->iProduct);
    m_sUID = get_string(handle, desc->iSerialNumber);

    setDeviceCategory(tr("USB Controller"));

    setInputDevice(true);
    setOutputDevice(true);
    m_pReader = nullptr;
}

BulkController::~BulkController() {
    if (isOpen()) {
        close();
    }
}

QString BulkController::mappingExtension() {
    return BULK_MAPPING_EXTENSION;
}

void BulkController::setMapping(std::shared_ptr<LegacyControllerMapping> pMapping) {
    m_pMapping = downcastAndTakeOwnership<LegacyHidControllerMapping>(std::move(pMapping));
}

std::shared_ptr<LegacyControllerMapping> BulkController::cloneMapping() {
    if (!m_pMapping) {
        return nullptr;
    }
    return m_pMapping->clone();
}

bool BulkController::matchMapping(const MappingInfo& mapping) {
    const QList<ProductInfo>& products = mapping.getProducts();
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
    if (!ok || m_vendorId != value) {
        return false;
    }
    value = product.product_id.toInt(&ok, 16);
    if (!ok || m_productId != value) {
        return false;
    }

#if defined(__WINDOWS__) || defined(__APPLE__)
    value = product.interface_number.toInt(&ok, 16);
    if (!ok || m_interfaceNumber != static_cast<unsigned int>(value)) {
        return false;
    }
#endif

    // Match found
    return true;
}

int BulkController::open() {
    if (isOpen()) {
        qCWarning(m_logBase) << "USB Bulk device" << getName() << "already open";
        return -1;
    }

    /* Look up endpoint addresses in supported database */
    int i;
    for (i = 0; bulk_supported[i].vendor_id; ++i) {
        if ((bulk_supported[i].vendor_id == m_vendorId) &&
                (bulk_supported[i].product_id == m_productId)) {
            m_inEndpointAddr = bulk_supported[i].in_epaddr;
            m_outEndpointAddr = bulk_supported[i].out_epaddr;
#if defined(__WINDOWS__) || defined(__APPLE__)
            m_interfaceNumber = bulk_supported[i].interface_number;
#endif
            break;
        }
    }

    if (bulk_supported[i].vendor_id == 0) {
        qCWarning(m_logBase) << "USB Bulk device" << getName() << "unsupported";
        return -1;
    }

    // XXX: we should enumerate devices and match vendor, product, and serial
    if (m_phandle == nullptr) {
        m_phandle = libusb_open_device_with_vid_pid(
                m_context, m_vendorId, m_productId);
    }

    if (m_phandle == nullptr) {
        qCWarning(m_logBase) << "Unable to open USB Bulk device" << getName();
        return -1;
    }

#if defined(__WINDOWS__) || defined(__APPLE__)
    if (m_interfaceNumber && libusb_kernel_driver_active(m_phandle, m_interfaceNumber) == 1) {
        qCDebug(m_logBase) << "Found a driver active for" << getName();
        if (libusb_detach_kernel_driver(m_phandle, 0) == 0)
            qCDebug(m_logBase) << "Kernel driver detached for" << getName();
        else {
            qCWarning(m_logBase) << "Couldn't detach kernel driver for" << getName();
            libusb_close(m_phandle);
            return -1;
        }
    }

    if (m_interfaceNumber) {
        int ret = libusb_claim_interface(m_phandle, m_interfaceNumber);
        if (ret < 0) {
            qCWarning(m_logBase) << "Cannot claim interface for" << getName()
                                 << ":" << libusb_error_name(ret);
            libusb_close(m_phandle);
            return -1;
        } else {
            qCDebug(m_logBase) << "Claimed interface for" << getName();
        }
    }
#endif

    setOpen(true);
    startEngine();

    if (m_pReader != nullptr) {
        qCWarning(m_logBase) << "BulkReader already present for" << getName();
    } else if (m_pMapping &&
            !(m_pMapping->getDeviceDirection() &
                    LegacyControllerMapping::DeviceDirection::Incoming)) {
        qDebug() << "The mapping for the bulk device" << getName()
                 << "doesn't require reading the data. Ignoring BulkReader "
                    "setup.";
    } else {
        m_pReader = new BulkReader(m_phandle, m_inEndpointAddr);
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
        qCWarning(m_logBase) << " device" << getName() << "already closed";
        return -1;
    }

    qCInfo(m_logBase) << "Shutting down USB Bulk device" << getName();

    // Stop the reading thread
    if (m_pReader == nullptr &&
            m_pMapping->getDeviceDirection() &
                    LegacyControllerMapping::DeviceDirection::Incoming) {
        qCWarning(m_logBase) << "BulkReader not present for" << getName()
                             << "yet the device is open!";
    } else if (m_pReader) {
        disconnect(m_pReader, &BulkReader::incomingData, this, &BulkController::receive);
        m_pReader->stop();
        qCInfo(m_logBase) << "  Waiting on reader to finish";
        m_pReader->wait();
        delete m_pReader;
        m_pReader = nullptr;
    }

    // Stop controller engine here to ensure it's done before the device is
    // closed in case it has any final parting messages
    stopEngine();

    // Close device
#if defined(__WINDOWS__) || defined(__APPLE__)
    if (m_interfaceNumber) {
        int ret = libusb_release_interface(m_phandle, m_interfaceNumber);
        if (ret < 0) {
            qCWarning(m_logBase) << "Cannot release interface for" << getName()
                                 << ":" << libusb_error_name(ret);
        }
    }
#endif
    qCInfo(m_logBase) << "  Closing device";
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
    VERIFY_OR_DEBUG_ASSERT(!m_pMapping ||
            m_pMapping->getDeviceDirection() &
                    LegacyControllerMapping::DeviceDirection::Outgoing) {
        qDebug() << "The mapping for the bulk device" << getName()
                 << "doesn't require sending data. Ignoring sending request.";
        return;
    }

    int ret;
    int transferred;

    // XXX: don't get drunk again.
    ret = libusb_bulk_transfer(m_phandle,
            m_outEndpointAddr,
            (unsigned char*)data.constData(),
            data.size(),
            &transferred,
            0);
    if (ret < 0) {
        qCWarning(m_logOutput) << "Unable to send data to" << getName()
                               << "serial #" << m_sUID << "-" << libusb_error_name(ret);
    } else {
        qCDebug(m_logOutput) << transferred << "bytes sent to" << getName()
                             << "serial #" << m_sUID;
    }
}
