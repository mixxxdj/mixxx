#pragma once

#include <QAtomicInt>
#include <QThread>

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

    void setMapping(std::shared_ptr<LegacyControllerMapping> pMapping) override;

    QList<LegacyControllerMapping::ScriptFileInfo> getMappingScriptFiles() override;
    QList<std::shared_ptr<AbstractLegacyControllerSetting>> getMappingSettings() override;

    bool isMappable() const override {
        if (!m_pMapping) {
            return false;
        }
        return m_pMapping->isMappable();
    }

    bool matchMapping(const MappingInfo& mapping) override;

  protected:
    void send(const QList<int>& data, unsigned int length) override;

  private:
    int open() override;
    int close() override;

    // For devices which only support a single report, reportID must be set to
    // 0x0.
    void sendBytes(const QByteArray& data) override;

    bool matchProductInfo(const ProductInfo& product);

    libusb_context* m_context;
    libusb_device_handle *m_phandle;

    // Local copies of things we need from desc

    unsigned short m_vendorId;
    unsigned short m_productId;
    unsigned char m_inEndpointAddr;
    unsigned char m_outEndpointAddr;
#if defined(__WINDOWS__) || defined(__APPLE__)
    unsigned int m_interfaceNumber;
#endif
    QString m_manufacturer;
    QString m_product;

    QString m_sUID;
    BulkReader* m_pReader;
    std::unique_ptr<LegacyHidControllerMapping> m_pMapping;
};
