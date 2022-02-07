#pragma once

#include <QAtomicInteger>
#include <QThread>
#include <map>

#include "controllers/controller.h"
#include "controllers/hid/hiddevice.h"
#include "controllers/hid/hidioreport.h"
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

    /// Sets the state of the HidIoThread lifecycle,
    /// if the previous state was not the expected,
    /// it returns with false as result.
    [[nodiscard]] bool testAndSetThreadState(HidIoThreadState expectedState,
            HidIoThreadState newState);

    /// Set the state of the HidIoThread lifecycles unconditional
    void setThreadState(HidIoThreadState expectedState);

    /// Wait's until the expected thread state, or the specified timeout, is reached.
    /// Returns immediately with true if the expected state is detected.
    [[nodiscard]] bool waitForThreadState(
            HidIoThreadState expectedState, unsigned int timeoutMillis);

    void latchOutputReport(const QByteArray& reportData, unsigned int reportID);
    QByteArray getInputReport(unsigned int reportID);
    void sendFeatureReport(const QByteArray& reportData, unsigned int reportID);
    QByteArray getFeatureReport(unsigned int reportID);

  signals:
    /// Signals that a HID InputReport received by Interrupt triggered from HID device
    void receive(const QByteArray& data, mixxx::Duration timestamp);

  private:
    bool sendNextOutputReport();

    void pollBufferedInputReports();
    void processInputReport(int bytesRead);

    const mixxx::hid::DeviceInfo m_deviceInfo;
    const RuntimeLoggingCategory m_logBase;
    const RuntimeLoggingCategory m_logInput;
    const RuntimeLoggingCategory m_logOutput;

    /// Mutex must be locked when using the m_pPollData, m_lastPollSize, m_pollingBufferIndex
    /// or the m_pHidDevice structure, which is not thread-safe for all hidapi backends.
    QMutex m_hidDeviceMutex;

    /// const pointer to the C data structure, which hidapi uses for communication between functions
    hid_device* const
            m_pHidDevice;

    static constexpr int kNumBuffers = 2;
    static constexpr int kBufferSize = 255;
    unsigned char m_pPollData[kNumBuffers][kBufferSize];
    int m_lastPollSize;
    int m_pollingBufferIndex;

    /// Must be locked when a operation changes the size of the m_outputReports map,
    /// or when modify the m_outputReportIterator
    QMutex m_outputReportMapMutex;

    typedef std::map<unsigned char, std::unique_ptr<HidIoReport>> OutputReportMap;
    OutputReportMap m_outputReports;
    OutputReportMap::iterator m_outputReportIterator;

    /// State of the HidIoThread lifecycle
    QAtomicInt m_state;
};
