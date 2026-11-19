#include "controllers/bulk/bulkenumerator.h"

#include <libusb.h>
#include <qglobal.h>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QXmlStreamReader>

#ifdef __ANDROID__
#include <QtCore/private/qandroidextras_p.h>
#include <android/api-level.h>
#include <android/log.h>
#include <jni.h>

#include <QJniObject>
#include <QtJniTypes>
#endif

#include "controllers/bulk/bulkcontroller.h"
#include "controllers/defs_controllers.h"
#include "moc_bulkenumerator.cpp"
#include "util/assert.h"

#ifndef __ANDROID__
BulkEnumerator::BulkEnumerator(const QStringList& mappingSearchPaths)
        : ControllerEnumerator(),
          m_pContext(nullptr) {
    loadSupportedDevices(mappingSearchPaths);

    int r;
    r = libusb_init(&m_pContext);
    VERIFY_OR_DEBUG_ASSERT(r == 0) {
        qCritical() << "libusb_init failed" << libusb_error_name(r);
        m_pContext = nullptr;
    }
}
#else
BulkEnumerator::BulkEnumerator(const QStringList& mappingSearchPaths)
        : ControllerEnumerator() {
    loadSupportedDevices(mappingSearchPaths);
}
#endif

BulkEnumerator::~BulkEnumerator() {
    qDebug() << "Deleting USB Bulk devices...";
    while (m_devices.size() > 0) {
        delete m_devices.takeLast();
    }
#ifndef __ANDROID__
    if (m_pContext) {
        libusb_exit(m_pContext);
    }
#endif
}

// Read the product info from a supported bulk device match
void BulkEnumerator::readProductInfo(QFile& file) {
    QXmlStreamReader xml(&file);

    while (!xml.atEnd()) {
        QXmlStreamReader::TokenType token = xml.readNext();

        // Only parse the <info> section; stop at its end element to
        // avoid parsing the entire (potentially large) mapping file.
        if (token == QXmlStreamReader::EndElement &&
                xml.name() == QStringLiteral("info")) {
            break;
        }

        if (token != QXmlStreamReader::StartElement ||
                xml.name() != QStringLiteral("product")) {
            continue;
        }

        const QXmlStreamAttributes attrs = xml.attributes();
        if (attrs.value(QStringLiteral("protocol")) != QStringLiteral("bulk")) {
            continue;
        }

        bool okVendor = false;
        bool okProduct = false;

        const auto vendorId = static_cast<std::uint16_t>(
                attrs.value(QStringLiteral("vendor_id")).toString().toUInt(&okVendor, 16));
        const auto productId = static_cast<std::uint16_t>(
                attrs.value(QStringLiteral("product_id")).toString().toUInt(&okProduct, 16));

        if (okVendor && okProduct) {
            m_supportedDevices.insert({vendorId, productId});
        }
    }
}

// Build the set of supported bulk devices (vendor id, product id) from the
// controller mapping XML files. Each <product protocol="bulk"> element
// declares one supported device.
void BulkEnumerator::loadSupportedDevices(const QStringList& searchPaths) {
    for (const QString& dirPath : searchPaths) {
        QDirIterator it(dirPath);

        while (it.hasNext()) {
            it.next();

            const QFileInfo fileInfo = it.fileInfo();
            const QString path = fileInfo.filePath();

            if (!path.endsWith(BULK_MAPPING_EXTENSION, Qt::CaseInsensitive)) {
                continue;
            }

            QFile file(path);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                readProductInfo(file);
                file.close();
            }
        }
    }
}

bool BulkEnumerator::is_interesting(std::uint16_t idVendor, std::uint16_t idProduct) {
    return m_supportedDevices.contains({idVendor, idProduct});
}

QList<Controller*> BulkEnumerator::queryDevices() {
    qDebug() << "Scanning USB Bulk devices:";
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
        return {};
    }

    QJniObject deviceListObject =
            usbManager.callMethod<QJniObject>("getDeviceList", "()Ljava/util/HashMap;");
    deviceListObject = deviceListObject.callMethod<jobject>("values", "()Ljava/util/Collection;");
    QJniArray<QJniObject> deviceList = QJniArray<QJniObject>(
            deviceListObject.callMethod<jobjectArray>("toArray"));
    __android_log_print(ANDROID_LOG_VERBOSE,
            "mixxx",
            "found %d USB devices for BULK enumerator",
            deviceList.size());

    for (const auto& pUsbDevice : deviceList) {
        const uint16_t idVendor = static_cast<unsigned short>(
                pUsbDevice->callMethod<jint>("getVendorId"));
        ;
        const uint16_t idProduct = static_cast<unsigned short>(
                pUsbDevice->callMethod<jint>("getProductId"));
        ;
        if (is_interesting(idVendor, idProduct)) {
            BulkController* pCurrentDevice =
                    new BulkController(pUsbDevice);
            m_devices.push_back(pCurrentDevice);
        }
    }
#else
    VERIFY_OR_DEBUG_ASSERT(m_pContext) {
        return {};
    }
    libusb_device** ppList;
    ssize_t cnt = libusb_get_device_list(m_pContext, &ppList);
    ssize_t i = 0;
    int err = 0;

    for (i = 0; i < cnt; i++) {
        libusb_device* pDevice = ppList[i];
        struct libusb_device_descriptor desc;

        libusb_get_device_descriptor(pDevice, &desc);
        if (is_interesting(desc.idVendor, desc.idProduct)) {
            struct libusb_device_handle* pHandle = nullptr;
            err = libusb_open(pDevice, &pHandle);
            if (err) {
                qWarning() << "Error opening a device:" << libusb_error_name(err);
                continue;
            }

            BulkController* pCurrentDevice =
                    new BulkController(m_pContext, pHandle, &desc);
            m_devices.push_back(pCurrentDevice);
        }
    }
    libusb_free_device_list(ppList, 1);
#endif
    return m_devices;
}
