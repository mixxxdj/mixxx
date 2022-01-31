#pragma once

#include <QThread>
#include <map>

#include "controllers/controller.h"
#include "controllers/hid/hiddevice.h"
#include "controllers/hid/hidioreport.h"
#include "util/compatibility/qatomic.h"
#include "util/compatibility/qmutex.h"
#include "util/duration.h"

enum class HidIoThreadState {
    Initialized,
    OutputActive,
    InputOutputActive,
    StopWhenAllReportsSent,
    StopRequested,
    Stopped,
};

class HidIoThread : public QThread {
    Q_OBJECT
  public:
    HidIoThread(hid_device* pDevice,
            const mixxx::hid::DeviceInfo& deviceInfo);
    ~HidIoThread() override;

    void run() override;
    QAtomicInt m_state;

    void latchOutputReport(const QByteArray& reportData, unsigned int reportID);
    QByteArray getInputReport(unsigned int reportID);
    void sendFeatureReport(const QByteArray& reportData, unsigned int reportID);
    QByteArray getFeatureReport(unsigned int reportID);

  signals:
    /// Signals that a HID InputReport received by Interrupt triggered from HID device
    void receive(const QByteArray& data, mixxx::Duration timestamp);

  private:
    static constexpr int kNumBuffers = 2;
    static constexpr int kBufferSize = 255;
    unsigned char m_pPollData[kNumBuffers][kBufferSize];
    int m_lastPollSize;
    int m_pollingBufferIndex;
    const RuntimeLoggingCategory m_logBase;
    const RuntimeLoggingCategory m_logInput;
    const RuntimeLoggingCategory m_logOutput;

    bool sendNextOutputReport();

    void pollBufferedInputReports();
    void processInputReport(int bytesRead);

    /// const pointer to the C data structure, which hidapi uses for communication between functions
    hid_device* const
            m_pHidDevice;
    const mixxx::hid::DeviceInfo m_deviceInfo;
    std::map<unsigned char, std::unique_ptr<HidIoReport>> m_outputReports;
    std::map<unsigned char, std::unique_ptr<HidIoReport>>::iterator m_OutputReportIterator;

    /// Must be locked when operation modify or depend on the size of the m_outputReports map
    QMutex m_outputReportMapMutex;

    /// Must be locked when using the m_pPollData, m_lastPollSize, m_pollingBufferIndex
    /// or the m_pHidDevice structure, which is not thread-safe for all hidapi backends.
    QMutex m_hidDeviceMutex;
};
