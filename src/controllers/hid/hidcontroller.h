#pragma once

#include <QThread>

#include "controllers/controller.h"
#include "controllers/hid/hiddevice.h"
#include "controllers/hid/hidiothread.h"
#include "controllers/hid/legacyhidcontrollermapping.h"
#include "util/duration.h"

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
    void sendBytes(const QByteArray& data) override;

    const mixxx::hid::DeviceInfo m_deviceInfo;

    std::unique_ptr<HidIoThread> m_pHidIoThread;
    std::shared_ptr<LegacyHidControllerMapping> m_pMapping;

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
        this->send(dataList, 0, 0);
    }

    /// @brief Sends HID OutputReport to HID device
    /// @param dataList Data to send as list of bytes
    /// @param length Unused but mandatory argument
    /// @param reportID 1...255 for HID devices that uses ReportIDs - or 0 for devices, which don't use ReportIDs
    /// @param skipIdenticalReports If set, the sending of the report is inhibited, when the data didn't changed
    Q_INVOKABLE void send(const QList<int>& dataList, unsigned int length, unsigned int reportID, bool skipIdenticalReports = true) {
        Q_UNUSED(length);
        QByteArray dataArray;
        dataArray.reserve(dataList.size());
        for (int datum : dataList) {
            dataArray.append(datum);
        }
        this->sendOutputReport(dataArray, reportID, skipIdenticalReports);
    }

    /// @brief Sends an OutputReport to HID device
    /// @param dataArray Data to send as byte array (Javascript type Uint8Array)
    /// @param reportID 1...255 for HID devices that uses ReportIDs - or 0 for devices, which don't use ReportIDs
    /// @param skipIdenticalReports If set, the sending of the report is inhibited, when the data didn't changed
    Q_INVOKABLE void sendOutputReport(const QByteArray& dataArray, unsigned int reportID, bool skipIdenticalReports = true) {
        VERIFY_OR_DEBUG_ASSERT(m_pHidController->m_pHidIoThread) {
            return;
        }
        m_pHidController->m_pHidIoThread->updateCachedOutputReportData(dataArray, reportID, skipIdenticalReports);
    }

    /// @brief getInputReport receives an InputReport from the HID device on request.
    /// @details This can be used on startup to initialize the knob positions in Mixxx
    ///          to the physical position of the hardware knobs on the controller.
    ///          The returned data structure for the input reports is the same
    ///          as in the polling functionality (including ReportID in first byte).
    ///          The returned list can be used to call the incomingData
    ///          function of the common-hid-packet-parser.
    ///          This is an optional command in the HID standard - not all devices support it.
    /// @param reportID 1...255 for HID devices that uses ReportIDs - or 0 for devices, which don't use
    /// @return 
    Q_INVOKABLE QByteArray getInputReport(
            unsigned int reportID) {
        VERIFY_OR_DEBUG_ASSERT(m_pHidController->m_pHidIoThread) {
            return {};
        }
        return m_pHidController->m_pHidIoThread->getInputReport(reportID);
    }

    /// @brief Sends a FeatureReport to HID device
    /// @param reportData Data to send as byte array (Javascript type Uint8Array)
    /// @param reportID 1...255 for HID devices that uses ReportIDs - or 0 for devices, which don't use
    Q_INVOKABLE void sendFeatureReport(
            const QByteArray& reportData, unsigned int reportID) {
        VERIFY_OR_DEBUG_ASSERT(m_pHidController->m_pHidIoThread) {
            return;
        }
        m_pHidController->m_pHidIoThread->sendFeatureReport(reportData, reportID);
    }

    
    /// @brief getFeatureReport receives a FeatureReport from the HID device on request.
    /// @param reportID 1...255 for HID devices that uses ReportIDs - or 0 for devices, which don't use
    /// @return The returned array matches the input format of sendFeatureReport (Javascript type Uint8Array),
    ///         allowing it to be read, modified and sent it back to the controller.
    Q_INVOKABLE QByteArray getFeatureReport(
            unsigned int reportID) {
        VERIFY_OR_DEBUG_ASSERT(m_pHidController->m_pHidIoThread) {
            return {};
        }
        return m_pHidController->m_pHidIoThread->getFeatureReport(reportID);
    }

  private:
    HidController* m_pHidController;
};
