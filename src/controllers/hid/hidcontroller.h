#pragma once

#include "controllers/controller.h"
#include "controllers/hid/hiddevice.h"
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

  protected:
    void sendReport(QList<int> data, unsigned int length, unsigned int reportID);

  private slots:
    int open() override;
    int close() override;

    bool poll() override;

  private:
    bool isPolling() const override;
    void processInputReport(int bytesRead);

    // For devices which only support a single report, reportID must be set to
    // 0x0.
    void sendBytes(const QByteArray& data) override;
    void sendBytesReport(QByteArray data, unsigned int reportID);
    void sendFeatureReport(const QList<int>& dataList, unsigned int reportID);

    // getInputReport receives an input report on request.
    // This can be used on startup to initialize the knob positions in Mixxx
    // to the physical position of the hardware knobs on the controller.
    // The returned data structure for the input reports is the same
    // as in the polling functionality (including ReportID in first byte).
    // The returned list can be used to call the incomingData
    // function of the common-hid-packet-parser.
    QList<int> getInputReport(unsigned int reportID);

    // getFeatureReport receives a feature reports on request.
    // HID doesn't support polling feature reports, therefore this is the
    // only method to get this information.
    // Usually, single bits in a feature report need to be set without
    // changing the other bits. The returned list matches the input
    // format of sendFeatureReport, allowing it to be read, modified
    // and sent it back to the controller.
    QList<int> getFeatureReport(unsigned int reportID);

    const mixxx::hid::DeviceInfo m_deviceInfo;

    hid_device* m_pHidDevice;
    std::shared_ptr<LegacyHidControllerMapping> m_pMapping;

    static constexpr int kNumBuffers = 2;
    static constexpr int kBufferSize = 255;
    unsigned char m_pPollData[kNumBuffers][kBufferSize];
    int m_lastPollSize;
    int m_pollingBufferIndex;

    friend class HidControllerJSProxy;
};

class HidControllerJSProxy : public ControllerJSProxy {
    Q_OBJECT
  public:
    HidControllerJSProxy(HidController* m_pController)
            : ControllerJSProxy(m_pController),
              m_pHidController(m_pController) {
    }

    Q_INVOKABLE void send(const QList<int>& data, unsigned int length = 0) override {
        m_pHidController->send(data, length);
    }

    Q_INVOKABLE void send(const QList<int>& data, unsigned int length, unsigned int reportID) {
        m_pHidController->sendReport(data, length, reportID);
    }

    Q_INVOKABLE QList<int> getInputReport(
            unsigned int reportID) {
        return m_pHidController->getInputReport(reportID);
    }

    Q_INVOKABLE void sendFeatureReport(
            const QList<int>& dataList, unsigned int reportID) {
        m_pHidController->sendFeatureReport(dataList, reportID);
    }

    Q_INVOKABLE QList<int> getFeatureReport(
            unsigned int reportID) {
        return m_pHidController->getFeatureReport(reportID);
    }

  private:
    HidController* m_pHidController;
};
