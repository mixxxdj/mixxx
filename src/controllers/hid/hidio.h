#pragma once

#include <QThread>
#include <map>

#include "controllers/controller.h"
#include "controllers/hid/hiddevice.h"
#include "util/compatibility/qatomic.h"
#include "util/duration.h"

class HidIoReport {
  public:
    HidIoReport(const unsigned char& reportId,
            hid_device* device,
            const mixxx::hid::DeviceInfo&& deviceInfo,
            const RuntimeLoggingCategory& logOutput);
    void sendOutputReport(QByteArray data);

  private:
    const unsigned char m_reportId;
    hid_device* const
            m_pHidDevice; // const pointer to the C data structure, which hidapi uses for communication between functions
    const mixxx::hid::DeviceInfo m_deviceInfo;
    const RuntimeLoggingCategory m_logOutput;
    QByteArray m_lastSentOutputReport;
};

class HidIoThread : public QThread {
    Q_OBJECT
  public:
    HidIoThread(hid_device* device,
            const mixxx::hid::DeviceInfo&& deviceInfo,
            const RuntimeLoggingCategory& logBase,
            const RuntimeLoggingCategory& logInput,
            const RuntimeLoggingCategory& logOutput);

    void stop() {
        atomicStoreRelaxed(m_stop, 1);
    }

    static constexpr int kNumBuffers = 2;
    static constexpr int kBufferSize = 255;
    unsigned char m_pPollData[kNumBuffers][kBufferSize];
    int m_lastPollSize;
    int m_pollingBufferIndex;
    const RuntimeLoggingCategory m_logBase;
    const RuntimeLoggingCategory m_logInput;
    const RuntimeLoggingCategory m_logOutput;

  signals:
    /// Signals that a HID InputReport received by Interrupt triggered from HID device
    void receive(const QByteArray& data, mixxx::Duration timestamp);

  public slots:
    QByteArray getInputReport(unsigned int reportID);
    void sendOutputReport(const QByteArray& reportData, unsigned int reportID);
    void sendFeatureReport(const QByteArray& reportData, unsigned int reportID);
    QByteArray getFeatureReport(unsigned int reportID);

  protected:
    void run();

  private:
    void poll();
    void processInputReport(int bytesRead);
    hid_device* const
            m_pHidDevice; // const pointer to the C data structure, which hidapi uses for communication between functions
    const mixxx::hid::DeviceInfo m_deviceInfo;
    QAtomicInt m_stop;
    std::map<unsigned char, std::unique_ptr<HidIoReport>> m_outputReports;
};
