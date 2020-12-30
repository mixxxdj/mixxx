#pragma once

#include <QAtomicInt>
#include <QThread>

#include "controllers/controller.h"
#include "controllers/hid/legacyhidcontrollermapping.h"
#include "controllers/hid/legacyhidcontrollermappingfilehandler.h"
#include "util/duration.h"

struct libusb_device_handle;
struct libusb_context;
struct libusb_device_descriptor;

/// USB Bulk controller backend
class BulkReader : public QThread {
    Q_OBJECT
  public:
    BulkReader(libusb_device_handle *handle, unsigned char in_epaddr);
    virtual ~BulkReader();

    void stop();

  signals:
    void incomingData(const QByteArray& data, mixxx::Duration timestamp);

  protected:
    void run();

  private:
    libusb_device_handle* m_phandle;
    QAtomicInt m_stop;
    unsigned char m_in_epaddr;
};

class BulkController : public Controller {
    Q_OBJECT
  public:
    BulkController(
            libusb_context* context,
            libusb_device_handle* handle,
            struct libusb_device_descriptor* desc);
    ~BulkController() override;

    QString mappingExtension() override;

    LegacyControllerMappingPointer getMapping() const override {
        LegacyHidControllerMapping* pClone = new LegacyHidControllerMapping();
        *pClone = m_mapping;
        return LegacyControllerMappingPointer(pClone);
    }

    void visit(const LegacyMidiControllerMapping* mapping) override;
    void visit(const LegacyHidControllerMapping* mapping) override;

    void accept(ControllerVisitor* visitor) override {
        if (visitor) {
            visitor->visit(this);
        }
    }

    bool isMappable() const override {
        return m_mapping.isMappable();
    }

    bool matchMapping(const MappingInfo& mapping) override;

  protected:
    void send(const QList<int>& data, unsigned int length) override;

  private slots:
    int open() override;
    int close() override;

  private:
    // For devices which only support a single report, reportID must be set to
    // 0x0.
    void sendBytes(const QByteArray& data) override;

    // Returns a pointer to the currently loaded controller mapping. For internal
    // use only.
    LegacyControllerMapping* mapping() override {
        return &m_mapping;
    }

    bool matchProductInfo(const ProductInfo& product);

    libusb_context* m_context;
    libusb_device_handle *m_phandle;

    // Local copies of things we need from desc

    unsigned short vendor_id;
    unsigned short product_id;
    unsigned char in_epaddr;
    unsigned char out_epaddr;
    QString manufacturer;
    QString product;

    QString m_sUID;
    BulkReader* m_pReader;
    LegacyHidControllerMapping m_mapping;
};
