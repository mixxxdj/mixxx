/**
  * @file bulkcontroller.cpp
  * @author Neale Pickett  neale@woozle.org
  * @date Thu Jun 28 2012
  * @brief USB Bulk controller backend
  *
  */

#include "controllers/bulk/bulkcontroller.h"
#include "controllers/bulk/bulksupported.h"

#include <time.h>
#include <stdio.h>
#ifdef NODUMP
#  define DUMPf(fmt, args...)
#else
#  define DUMPf(fmt, args...) fprintf(stderr, "%s:%s:%d " fmt "\n", __FILE__, __FUNCTION__, __LINE__, ##args)
#endif
#define DUMP() DUMPf("")
#define DUMP_d(v) DUMPf("%s = %d", #v, v)
#define DUMP_x(v) DUMPf("%s = 0x%x", #v, v)
#define DUMP_s(v) DUMPf("%s = %s", #v, v)
#define DUMP_c(v) DUMPf("%s = '%c' (0x%02x)", #v, v, v)
#define DUMP_p(v) DUMPf("%s = %p", #v, v)

BulkReader::BulkReader(libusb_device_handle *handle, unsigned char in_epaddr)
        : QThread() {
    m_phandle = handle;
    m_in_epaddr = in_epaddr;
}

BulkReader::~BulkReader() {
    m_phandle = NULL;
}

void BulkReader::run() {
    m_stop = 0;
    unsigned char data[255];

    DUMP();
    while (m_stop == 0) {
        // Blocked polling: The only problem with this is that we can't close
        // the device until the block is released, which means the controller
        // has to send more data
        //result = hid_read_timeout(m_pHidDevice, data, 255, -1);

        // This relieves that at the cost of higher CPU usage since we only
        // block for a short while (500ms)
        int transferred;
        int result;
        
        DUMP();
#if 0
        result = libusb_bulk_transfer(m_phandle,
                                      m_in_epaddr,
                                      data, sizeof data,
                                      &transferred, 500);
#else
        result = 0;
        sleep(1);
#endif
        DUMP();
        if (result > 0) {
            //qDebug() << "Read" << result << "bytes, pointer:" << data;
            QByteArray outData((char *)data, transferred);
            DUMP();
            emit(incomingData(outData));
        }
    }
}

static QString
get_string(libusb_device_handle *handle, u_int8_t id)
{
    unsigned char buf[128];
    QString s;

    DUMP();
    buf[0] = 0;
    if (id) {
        (void)libusb_get_string_descriptor_ascii(handle, id, buf, sizeof buf);
    } 

    return QString::fromAscii((char *)buf);
}


BulkController::BulkController(libusb_device_handle *handle,
                               struct libusb_device_descriptor *desc)
{
    DUMP();

    vendor_id = desc->idVendor;
    product_id = desc->idProduct;

    manufacturer = get_string(handle, desc->iManufacturer);
    product = get_string(handle, desc->iProduct);
    m_sUID = get_string(handle, desc->iSerialNumber);

    setDeviceCategory(tr("USB Controller"));

    setDeviceName(QString("%1 %2").arg(product).arg(m_sUID));

    setInputDevice(true);
    setOutputDevice(true);
    m_pReader = NULL;
}

BulkController::~BulkController() {
    close();
}

void BulkController::visit(const MidiControllerPreset* preset) {
    Q_UNUSED(preset);
    // TODO(XXX): throw a hissy fit.
    qDebug() << "ERROR: Attempting to load a MidiControllerPreset to an HidController!";
}

void BulkController::visit(const HidControllerPreset* preset) {
    DUMP();
    m_preset = *preset;
    // Emit presetLoaded with a clone of the preset.
    emit(presetLoaded(getPreset()));
}

bool BulkController::savePreset(const QString fileName) const {
    HidControllerPresetFileHandler handler;
    DUMP();
    return handler.save(m_preset, getName(), fileName);
}

bool BulkController::matchPreset(const PresetInfo& preset) {
    DUMP();
    const QList< QHash<QString,QString> > products = preset.getProducts();
    QHash <QString, QString> product;
    DUMP();
    foreach (product, products) {
        if (matchProductInfo(product))
            return true;
    }
    return false;
}

bool BulkController::matchProductInfo(QHash <QString,QString > info) {
    int value;
    bool ok;
    DUMP();
    // Product and vendor match is always required
    value = info["vendor_id"].toInt(&ok,16);
    if (!ok || vendor_id!=value) return false;
    value = info["product_id"].toInt(&ok,16);
    if (!ok || product_id!=value) return false;

    // Match found
    return true;
}

int BulkController::open() {
    int i;

    DUMP();
    if (isOpen()) {
        qDebug() << "USB Bulk device" << getName() << "already open";
        return -1;
    }

    /* Look up endpoint addresses in supported database */
    for (i = 0; bulk_supported[i].vendor_id; i += 1) {
        if ((bulk_supported[i].vendor_id == vendor_id) && 
            (bulk_supported[i].product_id == product_id)) {
            in_epaddr = bulk_supported[i].in_epaddr;
            out_epaddr = bulk_supported[i].out_epaddr;
            break;
        }
    }
    DUMP();

    if (bulk_supported[i].vendor_id == 0) {
        qWarning() << "USB Bulk device" << getName() << "unsupported";
        return -1;
    }
    DUMP();

    // XXX: we should enumerate devices and match vendor, product, and serial
    if (m_phandle == NULL) {
        m_phandle = libusb_open_device_with_vid_pid(NULL, vendor_id, product_id);
    }
    DUMP();

    if (m_phandle == NULL) {
        qWarning()  << "Unable to open USB Bulk device" << getName();
        return -1;
    }
    DUMP();

    setOpen(true);
    startEngine();

    DUMP();
    if (m_pReader != NULL) {
        qWarning() << "BulkReader already present for" << getName();
    } else {
        m_pReader = new BulkReader(m_phandle, in_epaddr);
        m_pReader->setObjectName(QString("BulkReader %1").arg(getName()));

        connect(m_pReader, SIGNAL(incomingData(QByteArray)),
                this, SLOT(receive(QByteArray)));

        // Controller input needs to be prioritized since it can affect the
        // audio directly, like when scratching
        m_pReader->start(QThread::HighPriority);
    }
    DUMP();

    return 0;
}

int BulkController::close() {
    if (!isOpen()) {
        qDebug() << " device" << getName() << "already closed";
        return -1;
    }

    qDebug() << "Shutting down USB Bulk device" << getName();

    // Stop the reading thread
    if (m_pReader == NULL) {
        qWarning() << "BulkReader not present for" << getName()
                   << "yet the device is open!";
    } else {
        disconnect(m_pReader, SIGNAL(incomingData(QByteArray)),
                   this, SLOT(receive(QByteArray)));
        m_pReader->stop();
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
    libusb_close(m_phandle);
    setOpen(false);
    return 0;
}

void BulkController::send(QList<int> data, unsigned int length) {
    Q_UNUSED(length);
    QByteArray temp;

    DUMP();
    foreach (int datum, data) {
        temp.append(datum);
    }
    send(temp);
}

void BulkController::send(QByteArray data) {
    int ret;
    int transferred;

    DUMP();
    // XXX: don't get drunk again.
    // XXX: it's dying on libusb_bulk_transfer calls.  Maybe m_phandel is NULL.
    DUMP_p(m_phandle);
    DUMP_d(out_epaddr);
    DUMP_s(data.constData());
    DUMP_d(data.size());
    DUMP_p(&transferred);
    ret = libusb_bulk_transfer(m_phandle, out_epaddr,
                               (unsigned char *)data.constData(), data.size(),
                               &transferred, 0);
    DUMP();
    if (ret < 0) {
        if (debugging()) {
            qWarning() << "Unable to send data to" << getName()
                       << "serial #" << m_sUID << ":"
                       << QString::fromAscii(libusb_error_name(ret));
        } else {
            qWarning() << "Unable to send data to" << getName() << ":"
                       << QString::fromAscii(libusb_error_name(ret));
        }
    } else if (debugging()) {
        qDebug() << ret << "bytes sent to" << getName()
                 << "serial #" << m_sUID;
    }
}
