#pragma once

#include <QAtomicInt>
#include <QList>
#include <QThread>
#include <mutex>
#include <optional>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

#include "controllers/controller.h"
#include "controllers/hid/legacyhidcontrollermapping.h"

struct libusb_device_handle;
struct libusb_context;
struct libusb_transfer;

/// A USB bulk endpoint with the size of a single transfer to/from it
struct BulkEndpoint {
    std::uint8_t address;
    int length;
};

/// USB Bulk controller backend
///
/// Reads from any number of input endpoints. One transfer per endpoint is kept
/// in flight and all of them are handled by the same thread.
class BulkReader : public QThread {
    Q_OBJECT
  public:
    BulkReader(libusb_device_handle* pHandle,
            libusb_context* pContext,
            const QList<BulkEndpoint>& inEndpoints);
    ~BulkReader() override;

    static void transferFinishedCb(libusb_transfer* pTransfer);
    void stop();

  signals:
    void incomingData(const QByteArray& data, mixxx::Duration timestamp);

  protected:
    void run() override;

  private:
    libusb_transfer* transfer_create(const BulkEndpoint& endpoint, unsigned int timeout);
    void handleTransfer(libusb_transfer* pTransfer);

    QAtomicInt m_stop;
    // Number of transfers currently submitted to libusb. Callbacks might be
    // invoked by any thread handling events on the shared libusb context.
    QAtomicInt m_activeTransfers;
    // Guards m_inTransfers, which stop() accesses from other threads
    std::mutex m_transfersMutex;
    QList<libusb_transfer*> m_inTransfers;
    libusb_context* m_pContext;
    libusb_device_handle* m_pHandle;
    const QList<BulkEndpoint> m_inEndpoints;
};

class BulkController : public Controller {
    Q_OBJECT
  public:
#ifndef Q_OS_ANDROID
    BulkController(
            libusb_context* pContext,
            libusb_device_handle* pHandle,
            struct libusb_device_descriptor* pDesc);
#else
    BulkController(
            const QJniObject& usbDevice);
#endif
    ~BulkController() override;

    QString mappingExtension() override;

    void setMapping(std::shared_ptr<LegacyControllerMapping> pMapping) override;

    QList<LegacyControllerMapping::ScriptFileInfo> getMappingScriptFiles() override;
    QList<std::shared_ptr<AbstractLegacyControllerSetting>> getMappingSettings() override;
#ifdef MIXXX_USE_QML
    QList<LegacyControllerMapping::QMLModuleInfo> getMappingModules() override;
    QList<LegacyControllerMapping::ScreenInfo> getMappingInfoScreens() override;
#endif

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

    uint8_t getInEndpointAddr() const {
        return m_inEndpointAddr;
    }

    uint8_t getOutEndpointAddr() const {
        return m_outEndpointAddr;
    }

    bool isMappable() const override {
        // On raw USB transfer level, there isn't any information about mappable controls
        return false;
    }

    bool matchMapping(const MappingInfo& mapping) override;

  protected:
    void send(const QList<int>& data, unsigned int length) override;

  private:
    int open(const QString& resourcePath) override;
    int close() override;

    // For devices which only support a single report, reportID must be set to
    // 0x0.
    bool sendBytes(const QByteArray& data) override;

    bool matchProductInfo(const ProductInfo& product);

    libusb_context* m_pContext;
    libusb_device_handle* m_pHandle;
#ifdef Q_OS_ANDROID
    QJniObject m_androidUsbDevice;
    QJniObject m_androidConnection;
#endif

    // Local copies of things we need from desc

    std::uint16_t m_vendorId;
    std::uint16_t m_productId;
    std::uint8_t m_inEndpointAddr;
    int m_inLength;
    std::uint8_t m_outEndpointAddr;
    std::optional<std::uint8_t> m_interfaceNumber;

    QString m_manufacturer;
    QString m_product;

    QString m_sUID;
    std::unique_ptr<BulkReader> m_pReader;
    std::unique_ptr<LegacyHidControllerMapping> m_pMapping;
};
