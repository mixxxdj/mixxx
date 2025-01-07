#pragma once

#include "controllers/controller.h"
#include "controllers/hid/hiddevice.h"
#include "controllers/hid/hidiothread.h"
#include "controllers/hid/legacyhidcontrollermapping.h"

/// HID controller backend
class HidController final : public Controller {
    Q_OBJECT
  public:
    explicit HidController(
            mixxx::hid::DeviceInfo&& deviceInfo);
    ~HidController() override;

    ControllerJSProxy* jsProxy() override;

    QString mappingExtension() override;

    virtual std::shared_ptr<LegacyControllerMapping> cloneMapping() override;
    void setMapping(std::shared_ptr<LegacyControllerMapping> pMapping) override;

    PhysicalTransportProtocol getPhysicalTransportProtocol() const override {
        return m_deviceInfo.getPhysicalTransportProtocol();
    }
    DataRepresentationProtocol getDataRepresentationProtocol() const override {
        return DataRepresentationProtocol::HID;
    }
    // Note, that for non-USB devices, the VendorString/ProductString can
    // contain the driver manufacturer, instead of the actual device
    // manufacturer. The VID/PID is a more reliable way to identify a HID
    // device.
    QString getVendorString() const override {
        return m_deviceInfo.getVendorString();
    }
    QString getProductString() const override {
        return m_deviceInfo.getProductString();
    }
    std::optional<uint16_t> getProductId() const override {
        return m_deviceInfo.getProductId();
    }
    std::optional<uint16_t> getVendorId() const override {
        return m_deviceInfo.getVendorId();
    }
    QString getSerialNumber() const override {
        return m_deviceInfo.getSerialNumber();
    }

    std::optional<uint8_t> getUsbInterfaceNumber() const override {
        return m_deviceInfo.getUsbInterfaceNumber();
    }
    uint16_t getUsagePage() const {
        return m_deviceInfo.getUsagePage();
    }

    uint16_t getUsage() const {
        return m_deviceInfo.getUsage();
    }

    QString getUsagePageDescription() const {
        return m_deviceInfo.getUsagePageDescription();
    }

    QString getUsageDescription() const {
        return m_deviceInfo.getUsageDescription();
    }
    bool isMappable() const override {
        if (!m_pMapping) {
            return false;
        }
        return m_pMapping->isMappable();
    }

    bool matchMapping(const MappingInfo& mapping) override;

  private slots:
    int open() override;
    int close() override;

  private:
    // For devices which only support a single report, reportID must be set to
    // 0x0.
    bool sendBytes(const QByteArray& data) override;

    const mixxx::hid::DeviceInfo m_deviceInfo;

    std::unique_ptr<HidIoThread> m_pHidIoThread;
    std::unique_ptr<LegacyHidControllerMapping> m_pMapping;

    friend class HidControllerJSProxy;
};

class HidControllerJSProxy : public ControllerJSProxy {
    Q_OBJECT
  public:
    HidControllerJSProxy(HidController* m_pController)
            : ControllerJSProxy(m_pController),
              m_pHidController(m_pController) {
    }

    /// @brief Sends HID OutputReport with hard coded ReportID 0 to HID device
    ///        This function only works with HID devices, which don't use ReportIDs
    /// @param dataList Data to send as list of bytes
    /// @param length Unused optional argument
    Q_INVOKABLE void send(const QList<int>& dataList, unsigned int length = 0) override {
        // This function is only for class compatibility with the (midi)controller
        Q_UNUSED(length);
        send(dataList, 0, 0);
    }

    /// @brief Sends HID OutputReport to HID device
    /// @param dataList Data to send as list of bytes
    /// @param length Unused but mandatory argument
    /// @param reportID 1...255 for HID devices that uses ReportIDs - or 0 for
    /// devices, which don't use ReportIDs
    /// @param useNonSkippingFIFO (optional)
    ///  Same as argument useNonSkippingFIFO of the sendOutputReport function,
    ///  which is documented below
    Q_INVOKABLE void send(const QList<int>& dataList,
            unsigned int length,
            quint8 reportID,
            bool useNonSkippingFIFO = false) {
        Q_UNUSED(length);
        QByteArray dataArray;
        dataArray.reserve(dataList.size());
        for (int datum : dataList) {
            dataArray.append(datum);
        }
        sendOutputReport(reportID, dataArray, useNonSkippingFIFO);
    }

    /// @brief Sends an OutputReport to HID device
    /// @param reportID 1...255 for HID devices that uses ReportIDs - or 0 for devices, which don't use ReportIDs
    /// @param dataArray Data to send as byte array (Javascript type Uint8Array)
    /// @param useNonSkippingFIFO (optional)
    ///  - False (default):
    ///    - Reports with identical data will be sent only once.
    ///    - If reports were superseded by newer data before they could be sent,
    ///      the oudated data will be skipped.
    ///    - This mode works for all USB HID class compatible reports,
    ///      in these each field represents the state of a control (e.g. an LED).
    ///    - This mode works best in overload situations, where more reports
    ///      are to be sent, than can be processed.
    ///  - True:
    ///    - The report will not be skipped under any circumstances,
    ///      except FIFO memory overflow.
    ///    - All reports with useNonSkippingFIFO set True will be send before
    ///      any cached report with useNonSkippingFIFO set False.
    ///    - All reports with useNonSkippingFIFO set True will be send in
    ///      strict First In / First Out (FIFO) order.
    ///    - Limit the use of this mode to the places, where it is really necessary.
    Q_INVOKABLE void sendOutputReport(quint8 reportID,
            const QByteArray& dataArray,
            bool useNonSkippingFIFO = false) {
        VERIFY_OR_DEBUG_ASSERT(m_pHidController->m_pHidIoThread) {
            return;
        }
        m_pHidController->m_pHidIoThread->updateCachedOutputReportData(
                reportID, dataArray, useNonSkippingFIFO);
    }

    /// @brief getInputReport receives an InputReport from the HID device on request.
    /// @details This can be used on startup to initialize the knob positions in Mixxx
    ///          to the physical position of the hardware knobs on the controller.
    ///          This is an optional command in the HID standard - not all devices support it.
    /// @param reportID 1...255 for HID devices that uses ReportIDs - or 0 for devices, which don't use
    /// @return Returns report data with ReportID byte as prefix
    Q_INVOKABLE QByteArray getInputReport(
            quint8 reportID) {
        VERIFY_OR_DEBUG_ASSERT(m_pHidController->m_pHidIoThread) {
            return {};
        }
        return m_pHidController->m_pHidIoThread->getInputReport(reportID);
    }

    /// @brief Sends a FeatureReport to HID device
    /// @param reportID 1...255 for HID devices that uses ReportIDs - or 0 for devices, which don't use
    /// @param reportData Data to send as byte array (Javascript type Uint8Array)
    Q_INVOKABLE void sendFeatureReport(
            quint8 reportID, const QByteArray& reportData) {
        VERIFY_OR_DEBUG_ASSERT(m_pHidController->m_pHidIoThread) {
            return;
        }
        m_pHidController->m_pHidIoThread->sendFeatureReport(reportID, reportData);
    }

    /// @brief getFeatureReport receives a FeatureReport from the HID device on request.
    /// @param reportID 1...255 for HID devices that uses ReportIDs - or 0 for devices, which don't use
    /// @return The returned array matches the input format of sendFeatureReport (Javascript type Uint8Array),
    ///         allowing it to be read, modified and sent it back to the controller.
    Q_INVOKABLE QByteArray getFeatureReport(
            quint8 reportID) {
        VERIFY_OR_DEBUG_ASSERT(m_pHidController->m_pHidIoThread) {
            return {};
        }
        return m_pHidController->m_pHidIoThread->getFeatureReport(reportID);
    }

  private:
    HidController* m_pHidController;
};
