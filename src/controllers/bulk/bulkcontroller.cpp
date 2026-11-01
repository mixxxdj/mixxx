#include "controllers/bulk/bulkcontroller.h"

#include <libusb.h>

#include <memory>

#include "controllers/defs_controllers.h"
#include "moc_bulkcontroller.cpp"
#include "util/cmdlineargs.h"
#include "util/time.h"

#if defined(Q_OS_ANDROID)
#include "controllers/android.h"
#endif

namespace {
/// Parse a USB endpoint address attribute (e.g. "0x82"). Returns 0 on failure.
std::uint8_t parseEndpointAddress(const QString& value) {
    bool ok = false;
    const uint v = value.toUInt(&ok, 16);

    return ok ? static_cast<std::uint8_t>(v) : 0;
}

/// Parse a bulk input endpoint length attribute (e.g. "255"). Defaults to 255
/// when the attribute is missing or invalid.
int parseEndpointLength(const QString& value) {
    if (value.isEmpty()) {
        return 255;
    }

    bool ok = false;
    const int v = value.toInt(&ok, 10);

    return ok ? v : 255;
}

/// Parse an optional USB interface number attribute (e.g. "0x04").
std::optional<std::uint8_t> parseInterfaceNumber(const QString& value) {
    if (value.isEmpty()) {
        return std::nullopt;
    }

    bool ok = false;
    const uint v = value.toUInt(&ok, 16);

    return ok ? std::make_optional(static_cast<std::uint8_t>(v)) : std::nullopt;
}
} // namespace

BulkReader::BulkReader(libusb_device_handle* handle,
        libusb_context* context,
        const QList<BulkEndpoint>& inEndpoints)
        : QThread(),
          m_stop(0),
          m_activeTransfers(0),
          m_pContext(context),
          m_pHandle(handle),
          m_inEndpoints(inEndpoints) {
}

BulkReader::~BulkReader() {
    // All transfers are freed by run() before it returns
    wait();
}

void BulkReader::transferFinishedCb(libusb_transfer* pTransfer) {
    static_cast<BulkReader*>(pTransfer->user_data)->handleTransfer(pTransfer);
}

void BulkReader::handleTransfer(libusb_transfer* pTransfer) {
    switch (pTransfer->status) {
    case LIBUSB_TRANSFER_COMPLETED:
    case LIBUSB_TRANSFER_TIMED_OUT:
        if (pTransfer->actual_length > 0) {
            QByteArray byteArray(reinterpret_cast<char*>(pTransfer->buffer),
                    pTransfer->actual_length);
            emit incomingData(byteArray, mixxx::Time::elapsed());
        }
        if (m_stop.loadAcquire() == 0) {
            const int error = libusb_submit_transfer(pTransfer);
            if (error == 0) {
                // Transfer stays in flight
                return;
            }
            qWarning() << "Cannot resubmit bulk transfer for endpoint"
                       << pTransfer->endpoint << ":" << libusb_error_name(error);
        }
        break;
    case LIBUSB_TRANSFER_CANCELLED:
    case LIBUSB_TRANSFER_ERROR:
    case LIBUSB_TRANSFER_NO_DEVICE:
    case LIBUSB_TRANSFER_STALL:
    case LIBUSB_TRANSFER_OVERFLOW:
        stop();
        break;
    }
    m_activeTransfers.deref();
}

libusb_transfer* BulkReader::transfer_create(const BulkEndpoint& endpoint, unsigned int timeout) {
    libusb_transfer* pTransfer = libusb_alloc_transfer(0);
    if (pTransfer == nullptr) {
        return nullptr;
    }

    // Using malloc here to make use of LIBUSB_TRANSFER_FREE_BUFFER
    auto* buffer = static_cast<unsigned char*>(std::malloc(
            static_cast<std::size_t>(endpoint.length) * sizeof(unsigned char)));
    if (buffer == nullptr) {
        libusb_free_transfer(pTransfer);
        return nullptr;
    }

    libusb_fill_bulk_transfer(pTransfer,
            m_pHandle,
            endpoint.address,
            buffer,
            endpoint.length,
            transferFinishedCb,
            this,
            timeout);

    // Automatically free the transfer buffer on exit
    pTransfer->flags = LIBUSB_TRANSFER_FREE_BUFFER;

    return pTransfer;
}

void BulkReader::stop() {
    if (!m_stop.testAndSetAcquire(0, 1)) {
        return;
    }

    qInfo() << "Cancelling bulk transfers";
    std::lock_guard<std::mutex> lock(m_transfersMutex);
    for (libusb_transfer* pTransfer : std::as_const(m_inTransfers)) {
        // Fails harmlessly for transfers which are not in flight
        libusb_cancel_transfer(pTransfer);
    }
}

void BulkReader::run() {
    {
        std::lock_guard<std::mutex> lock(m_transfersMutex);
        for (const auto& endpoint : m_inEndpoints) {
            libusb_transfer* pTransfer = transfer_create(endpoint, 500);
            if (pTransfer == nullptr) {
                qWarning() << "Cannot create a bulk transfer for endpoint"
                           << endpoint.address;
                m_stop.storeRelease(1);
                break;
            }
            m_inTransfers.append(pTransfer);

            // Count before submitting, the callback might fire immediately
            // on another thread handling events of the same context
            m_activeTransfers.ref();
            const int error = libusb_submit_transfer(pTransfer);
            if (error != 0) {
                qWarning() << "Cannot submit a bulk transfer for endpoint"
                           << endpoint.address << ":" << libusb_error_name(error);
                m_activeTransfers.deref();
                m_stop.storeRelease(1);
                break;
            }
        }

        if (m_stop.loadAcquire() != 0) {
            for (libusb_transfer* pTransfer : std::as_const(m_inTransfers)) {
                libusb_cancel_transfer(pTransfer);
            }
        }
    }

    // Keep handling events until every transfer has finished, including
    // the ones being cancelled after stop() was requested
    while (m_activeTransfers.loadAcquire() > 0) {
        struct timeval tv{0, 500000}; // 500ms timeout
        libusb_handle_events_timeout(m_pContext, &tv);
    }

    {
        std::lock_guard<std::mutex> lock(m_transfersMutex);
        for (libusb_transfer* pTransfer : std::as_const(m_inTransfers)) {
            libusb_free_transfer(pTransfer);
        }
        m_inTransfers.clear();
    }

    qDebug() << "Stopped Reader";
}

#ifndef Q_OS_ANDROID
static QString get_string(libusb_device_handle* pHandle, uint8_t id) {
    unsigned char buf[128] = { 0 };

    if (id) {
        libusb_get_string_descriptor_ascii(pHandle, id, buf, sizeof(buf));
    }

    return QString::fromLatin1((char*)buf);
}

BulkController::BulkController(libusb_context* pContext,
        libusb_device_handle* pHandle,
        struct libusb_device_descriptor* pDesc)
        : Controller(QString("%1 %2").arg(
                  get_string(pHandle, pDesc->iProduct),
                  get_string(pHandle, pDesc->iSerialNumber))),
          m_pContext(pContext),
          m_pHandle(pHandle),
          m_outEndpoint(std::nullopt) {
    m_vendorId = pDesc->idVendor;
    m_productId = pDesc->idProduct;

    m_manufacturer = get_string(pHandle, pDesc->iManufacturer);
    m_product = get_string(pHandle, pDesc->iProduct);
    m_sUID = get_string(pHandle, pDesc->iSerialNumber);

    setInputDevice(true);
    setOutputDevice(true);
}
#else
BulkController::BulkController(const QJniObject& usbDevice)
        : Controller(QString("%1 %2").arg(
                  usbDevice.callMethod<jstring>("getProductName").toString(),
                  usbDevice.callMethod<jstring>("getSerialNumber").toString())),
          m_pContext(nullptr),
          m_pHandle(nullptr),
          m_androidUsbDevice(usbDevice),
          m_outEndpoint(std::nullopt) {
    m_vendorId = static_cast<unsigned short>(usbDevice.callMethod<jint>("getVendorId"));
    m_productId = static_cast<unsigned short>(usbDevice.callMethod<jint>("getProductId"));

    m_manufacturer = usbDevice.callMethod<jstring>("getManufacturerName").toString();
    m_product = usbDevice.callMethod<jstring>("getProductName").toString();
    m_sUID = usbDevice.callMethod<jstring>("getSerialNumber").toString();
    if (m_sUID.isEmpty()) {
        // Android won't allow reading serial number if permission wasn't
        // granted previously. Is this an issue?
        m_sUID = "N/A";
    }

    setInputDevice(true);
    setOutputDevice(true);
}
#endif

BulkController::~BulkController() {
    if (isOpen()) {
        close();
    }
}

QString BulkController::mappingExtension() {
    return BULK_MAPPING_EXTENSION;
}

void BulkController::setMapping(std::shared_ptr<LegacyControllerMapping> pMapping) {
    m_pMutableMapping = pMapping;
    m_pMapping = downcastAndClone<LegacyHidControllerMapping>(pMapping.get());
}

QList<LegacyControllerMapping::ScriptFileInfo> BulkController::getMappingScriptFiles() {
    if (!m_pMapping) {
        return {};
    }
    return m_pMapping->getScriptFiles();
}

QList<std::shared_ptr<AbstractLegacyControllerSetting>> BulkController::getMappingSettings() {
    if (!m_pMapping) {
        return {};
    }
    return m_pMapping->getSettings();
}

#ifdef MIXXX_USE_QML
QList<LegacyControllerMapping::QMLModuleInfo> BulkController::getMappingModules() {
    if (!m_pMapping) {
        return {};
    }
    return m_pMapping->getModules();
}

QList<LegacyControllerMapping::ScreenInfo> BulkController::getMappingInfoScreens() {
    if (!m_pMapping) {
        return {};
    }
    return m_pMapping->getInfoScreens();
}
#endif

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

    // Match found
    return true;
}

int BulkController::open(const QString& resourcePath) {
    if (isOpen()) {
        qCWarning(m_logBase) << "USB Bulk device" << getName() << "already open";
        return -1;
    }

    /* Look up endpoint addresses in the controller mapping */

    const ProductInfo* pMatchedProduct = nullptr;
    if (m_pMapping) {
        for (const auto& product : m_pMapping->productMatch()) {
            bool ok = false;
            const std::uint16_t vendorId =
                    static_cast<std::uint16_t>(product.vendor_id.toUInt(&ok, 16));

            if (!ok || vendorId != m_vendorId) {
                continue;
            }

            ok = false;
            const std::uint16_t productId =
                    static_cast<std::uint16_t>(product.product_id.toUInt(&ok, 16));

            if (!ok || productId != m_productId) {
                continue;
            }

            pMatchedProduct = &product;

            break;
        }
    }

    if (!pMatchedProduct) {
        qCWarning(m_logBase) << "USB Bulk device" << getName()
                             << "has no matching product info in mapping";
        return -1;
    }

    m_interfaces = pMatchedProduct->bulkInterfaces;
    m_interfaceNumbers.clear();
    m_inEndpoints.clear();
    m_outEndpoint.reset();
    for (const auto& interfaceInfo : std::as_const(m_interfaces)) {
        const auto interfaceNumber = parseInterfaceNumber(interfaceInfo.number);
        if (!interfaceNumber.has_value()) {
            qCWarning(m_logBase) << "USB Bulk device" << getName()
                                 << "has invalid interface number"
                                 << interfaceInfo.number << "in its mapping";
            continue;
        }

        m_interfaceNumbers.append(*interfaceNumber);

        for (const auto& endpointInfo : interfaceInfo.endpoints) {
            const BulkEndpoint endpoint{parseEndpointAddress(endpointInfo.address),
                    parseEndpointLength(endpointInfo.length)};
            const bool isInEndpoint = endpointInfo.direction == QStringLiteral("in");
            const bool isOutEndpoint = endpointInfo.direction == QStringLiteral("out");
            // The direction must also match bit 7 of the endpoint address
            const bool hasInAddress = (endpoint.address & LIBUSB_ENDPOINT_IN) != 0;

            if (endpoint.address == 0 ||
                    !(isInEndpoint || isOutEndpoint) ||
                    isInEndpoint != hasInAddress) {
                qCWarning(m_logBase) << "USB Bulk device" << getName()
                                     << "has invalid" << endpointInfo.direction
                                     << "endpoint" << endpointInfo.address
                                     << "in its mapping";
                continue;
            }

            if (isInEndpoint) {
                m_inEndpoints.append(endpoint);
            } else if (m_outEndpoint.has_value()) {
                qCWarning(m_logBase) << "USB Bulk device" << getName()
                                     << "declares more than one out endpoint, ignoring"
                                     << endpointInfo.address;
            } else {
                m_outEndpoint = endpoint;
            }
        }
    }

#ifdef __ANDROID__
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

    if (!usbManager.callMethod<jboolean>("hasPermission",
                "(Landroid/hardware/usb/UsbDevice;)Z",
                m_androidUsbDevice)) {
        auto pendingIntent = mixxx::android::getIntent();
        usbManager.callMethod<void>("requestPermission",
                "(Landroid/hardware/usb/UsbDevice;Landroid/app/"
                "PendingIntent;)V",
                m_androidUsbDevice,
                pendingIntent);
        // Wait for permission
        if (!mixxx::android::waitForPermission(m_androidUsbDevice)) {
            qDebug() << "access to device wasn't granted";
            return -1;
        }
        m_sUID = m_androidUsbDevice.callMethod<jstring>("getSerialNumber").toString();
    }
    m_androidConnection = usbManager.callMethod<jobject>("openDevice",
            "(Landroid/hardware/usb/UsbDevice;)Landroid/hardware/usb/"
            "UsbDeviceConnection;",
            m_androidUsbDevice);

    if (!m_androidConnection.isValid()) {
        qDebug() << "Unable to open BULK device";
        return -1;
    }

    auto fileDescriptor = static_cast<intptr_t>(
            m_androidConnection.callMethod<jint>("getFileDescriptor"));

    // Open device by file descriptor
    qCInfo(m_logBase) << "Opening BULK device" << getName()
                      << "by file descriptor"
                      << fileDescriptor << "and interfaces"
                      << m_interfaceNumbers;

    libusb_set_option(nullptr, LIBUSB_OPTION_NO_DEVICE_DISCOVERY);
    libusb_init(&m_pContext);

    int error = libusb_wrap_sys_device(nullptr, fileDescriptor, &m_pHandle);
    if (error < 0) {
        qCWarning(m_logBase) << "Cannot open interface for" << getName()
                             << ":" << libusb_error_name(error);
        libusb_close(m_pHandle);
        return -1;
    } else {
        qCDebug(m_logBase) << "Opened interface for" << getName();
    }
#else
    // XXX: we should enumerate devices and match vendor, product, and serial
    if (m_pHandle == nullptr) {
        m_pHandle = libusb_open_device_with_vid_pid(
                m_pContext, m_vendorId, m_productId);
    }

    if (m_pHandle == nullptr) {
        qCWarning(m_logBase) << "Unable to open USB Bulk device" << getName();
        return -1;
    }
    if (libusb_set_auto_detach_kernel_driver(m_pHandle, true) == LIBUSB_ERROR_NOT_SUPPORTED) {
        qCDebug(m_logBase) << "unable to automatically detach kernel driver for" << getName();
    }
#endif

    for (const std::uint8_t interfaceNumber : std::as_const(m_interfaceNumbers)) {
        int error = libusb_claim_interface(m_pHandle, interfaceNumber);
        if (error < 0) {
            qCWarning(m_logBase) << "Cannot claim interface" << interfaceNumber
                                 << "for" << getName() << ":" << libusb_error_name(error);
            releaseInterfaces();
            libusb_close(m_pHandle);
            return -1;
        } else {
            qCDebug(m_logBase) << "Claimed interface" << interfaceNumber << "for" << getName();
            m_claimedInterfaceNumbers.append(interfaceNumber);
        }
    }

    startEngine();

    if (m_pReader != nullptr) {
        qCWarning(m_logBase) << "BulkReader already present for" << getName();
    } else if (m_pMapping &&
            !(m_pMapping->getDeviceDirection() &
                    LegacyControllerMapping::DeviceDirection::Incoming)) {
        qDebug() << "The mapping for the bulk device" << getName()
                 << "doesn't require reading the data. Ignoring BulkReader "
                    "setup.";
    } else if (!m_inEndpoints.isEmpty()) {
        m_pReader = std::make_unique<BulkReader>(
                m_pHandle, m_pContext, m_inEndpoints);
        m_pReader->setObjectName(QString("BulkReader %1").arg(getName()));

        connect(m_pReader.get(), &BulkReader::incomingData, this, &BulkController::receive);

        // Controller input needs to be prioritized since it can affect the
        // audio directly, like when scratching
        m_pReader->start(QThread::HighPriority);
    }
    applyMapping(resourcePath);
    setOpen(true);
    return 0;
}

int BulkController::close() {
    if (!isOpen()) {
        qCWarning(m_logBase) << " device" << getName() << "already closed";
        return -1;
    }

    qCInfo(m_logBase) << "Shutting down USB Bulk device" << getName();

    // Stop the reading thread
    if (m_pReader == nullptr && !m_inEndpoints.isEmpty() &&
            m_pMapping->getDeviceDirection() &
                    LegacyControllerMapping::DeviceDirection::Incoming) {
        qCWarning(m_logBase) << "BulkReader not present for" << getName()
                             << "yet the device is open!";
    } else if (m_pReader) {
        disconnect(m_pReader.get(), &BulkReader::incomingData, this, &BulkController::receive);
        m_pReader->stop();
        qCInfo(m_logBase) << "  Waiting on reader to finish";
        m_pReader->wait();
        m_pReader.reset();
    }

    // Stop controller engine here to ensure it's done before the device is
    // closed in case it has any final parting messages
    stopEngine();

    // Close device
    releaseInterfaces();
    qCInfo(m_logBase) << "  Closing device";
    libusb_close(m_pHandle);

#ifdef Q_OS_ANDROID
    if (m_androidConnection.isValid()) {
        m_androidConnection.callMethod<void>("close");
    }
#endif
    m_pHandle = nullptr;
    setOpen(false);
    return 0;
}

void BulkController::releaseInterfaces() {
    // Release in reverse order of claiming
    while (!m_claimedInterfaceNumbers.isEmpty()) {
        const std::uint8_t interfaceNumber = m_claimedInterfaceNumbers.takeLast();
        int error = libusb_release_interface(m_pHandle, interfaceNumber);
        if (error < 0) {
            qCWarning(m_logBase) << "Cannot release interface" << interfaceNumber
                                 << "for" << getName() << ":" << libusb_error_name(error);
        }
    }
}

void BulkController::send(const QList<int>& data, unsigned int length) {
    Q_UNUSED(length);
    QByteArray temp;

    foreach (int datum, data) {
        temp.append(datum);
    }
    sendBytes(temp);
}

bool BulkController::sendBytes(const QByteArray& data) {
    VERIFY_OR_DEBUG_ASSERT(!m_pMapping ||
            m_pMapping->getDeviceDirection() &
                    LegacyControllerMapping::DeviceDirection::Outgoing) {
        qDebug() << "The mapping for the bulk device" << getName()
                 << "doesn't require sending data. Ignoring sending request.";
        return false;
    }

    if (!m_outEndpoint.has_value()) {
        qCWarning(m_logOutput) << "The mapping for the bulk device" << getName()
                               << "doesn't declare an out endpoint. Ignoring sending request.";
        return false;
    }

    int ret;
    int transferred;

    // XXX: don't get drunk again.
    ret = libusb_bulk_transfer(m_pHandle,
            m_outEndpoint->address,
            (unsigned char*)data.constData(),
            data.size(),
            &transferred,
            5000 // Send timeout in milliseconds
    );
    if (ret < 0) {
        qCWarning(m_logOutput) << "Unable to send data to" << getName()
                               << "serial #" << m_sUID << "-" << libusb_error_name(ret);
        return false;
    } else if (CmdlineArgs::Instance().getControllerDebug()) {
        qCDebug(m_logOutput) << transferred << "bytes sent to" << getName()
                             << "serial #" << m_sUID;
    }
    return true;
}
