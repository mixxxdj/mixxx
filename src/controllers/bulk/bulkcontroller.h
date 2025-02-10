#pragma once

#include <QAtomicInt>
#include <QThread>
#include <optional>

#include "controllers/controller.h"
#include "controllers/hid/legacyhidcontrollermapping.h"

struct libusb_device_handle;
struct libusb_context;

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

    virtual std::shared_ptr<LegacyControllerMapping> cloneMapping() override;
    void setMapping(std::shared_ptr<LegacyControllerMapping> pMapping) override;

    PhysicalTransportProtocol getPhysicalTransportProtocol() const override {
        return PhysicalTransportProtocol::USB;
    }
    DataRepresentationProtocol getDataRepresentationProtocol() const override {
        return DataRepresentationProtocol::USB_BULK_TRANSFER;
    }

    QString getVendorString() const override {
        return m_manufacturer;
    }
    QString getProductString() const override {
        return m_product;
    }
    std::optional<uint16_t> getVendorId() const override {
        return m_vendorId;
    }
    std::optional<uint16_t> getProductId() const override {
        return m_productId;
    }
    QString getSerialNumber() const override {
        return m_sUID;
    }

    std::optional<uint8_t> getUsbInterfaceNumber() const override {
        return m_interfaceNumber;
    }

    bool isMappable() const override {
        // On raw USB transfer level, there isn't any information about mappable controls
        return false;
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
    bool sendBytes(const QByteArray& data) override;

    bool matchProductInfo(const ProductInfo& product);

    libusb_context* m_context;
    libusb_device_handle *m_phandle;

    // Local copies of things we need from desc

    std::uint16_t m_vendorId;
    std::uint16_t m_productId;
    std::uint8_t m_inEndpointAddr;
    std::uint8_t m_outEndpointAddr;
    std::optional<std::uint8_t> m_interfaceNumber;

    QString m_manufacturer;
    QString m_product;

    QString m_sUID;
    BulkReader* m_pReader;
    std::unique_ptr<LegacyHidControllerMapping> m_pMapping;
};
