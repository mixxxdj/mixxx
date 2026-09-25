#pragma once

#include <QSemaphore>
#include <QThread>
#include <map>

#include "controllers/hid/hiddevice.h"
#include "controllers/hid/hidioglobaloutputreportfifo.h"
#include "controllers/hid/hidiooutputreport.h"
#include "util/compatibility/qmutex.h"
#include "util/duration.h"
#include "util/runtimeloggingcategory.h"

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
            const mixxx::hid::DeviceInfo& deviceInfo,
            std::optional<bool> deviceUsesReportIds);
    ~HidIoThread() override;

    void run() override;

    /// Sets the state of the HidIoThread lifecycle,
    /// if the previous state was not the expected,
    /// it returns with false as result.
    [[nodiscard]] bool testAndSetThreadState(HidIoThreadState expectedState,
            HidIoThreadState newState);

    /// Set the state of the HidIoThread lifecycles unconditional
    void setThreadState(HidIoThreadState expectedState);

    /// Wait's until the run loop stopped, or the specified timeout, is reached.
    /// Returns immediately with true if the run loop is stopped.
    [[nodiscard]] bool waitUntilRunLoopIsStopped(unsigned int timeoutMillis);

    void updateCachedOutputReportData(quint8 reportID,
            const QByteArray& reportData,
            bool useNonSkippingFIFO);
    QByteArray getInputReport(quint8 reportID);
    void sendFeatureReport(quint8 reportID, const QByteArray& reportData);
    QByteArray getFeatureReport(quint8 reportID);

#ifdef Q_OS_ANDROID
    // On Android, we open a connection to the device in JNI. we must keep the
    // object alive and referenced to prevent GC and file descriptor being
    // closed
    void setDeviceConnection(QJniObject&& connection) {
        m_androidConnection = connection;
    }
#endif

  signals:
    /// Signals that a HID InputReport received by Interrupt triggered from HID device
    void receive(const QByteArray& data, mixxx::Duration timestamp);
    void reportReceived(quint8 reportId, const QByteArray& data);

  private:
    bool sendNextCachedOutputReport();

    void pollBufferedInputReports();
    void processInputReport(int bytesRead);

    const mixxx::hid::DeviceInfo m_deviceInfo;
    const RuntimeLoggingCategory m_logBase;
    const RuntimeLoggingCategory m_logInput;
    const RuntimeLoggingCategory m_logOutput;

    /// This mutex must be locked for any hid device operation using the m_pHidDevice structure.
    /// If the hid_error functions is called after the hid device operation to get the error message,
    /// this mutex must not be unlocked before hid_error.
    /// This mutex must be locked also, for access to m_pPollData, m_lastPollSize, m_pollingBufferIndex.
    QMutex m_hidDeviceAndPollMutex;

    /// const pointer to the C data structure, which hidapi uses for communication between functions
    hid_device* const
            m_pHidDevice;

    static constexpr int kNumBuffers = 2;
    static constexpr int kBufferSize = 255;
    unsigned char m_pPollData[kNumBuffers][kBufferSize];
    int m_lastPollSize;
    int m_pollingBufferIndex;
    bool m_hidReadErrorLogged;

    std::optional<bool> m_deviceUsesReportIds;

    /// Must be locked when a operation changes the size of the m_outputReports map,
    /// or when modify the m_outputReportIterator
    QMutex m_outputReportMapMutex;

    typedef std::map<unsigned char, std::unique_ptr<HidIoOutputReport>> OutputReportMap;
    /// m_outputReports is an empty map after class initialization.
    /// An entry is inserted each time, when an OutputReport is send for the first time.
    /// Until then, it's not known, which OutputReports a device/mapping has.
    /// No other modifications to the map are done, until destruction of this class.
    OutputReportMap m_outputReports;
    OutputReportMap::iterator m_outputReportIterator;

    HidIoGlobalOutputReportFifo m_globalOutputReportFifo;

    /// State of the HidIoThread lifecycle
    QAtomicInt m_state;

    /// Semaphore with capacity 1, which is left acquired, as long as the run loop of the thread runs
    QSemaphore m_runLoopSemaphore;
#ifdef Q_OS_ANDROID
    QJniObject m_androidConnection;
#endif
};
