#pragma once

#include <QThread>
#include <map>

#include "controllers/controller.h"
#include "controllers/hid/hiddevice.h"
#include "controllers/hid/hidioreport.h"
#include "util/compatibility/qatomic.h"
#include "util/compatibility/qmutex.h"
#include "util/duration.h"

class HidIoThread : public QThread {
    Q_OBJECT
  public:
    HidIoThread(hid_device* device,
            const mixxx::hid::DeviceInfo&& deviceInfo);

    void startPollTimer();
    void stopPollTimer();

    QByteArray getInputReport(unsigned int reportID);
    void sendFeatureReport(const QByteArray& reportData, unsigned int reportID);
    QByteArray getFeatureReport(unsigned int reportID);

  signals:
    /// Signals that a HID InputReport received by Interrupt triggered from HID device
    void receive(const QByteArray& data, mixxx::Duration timestamp);

  public slots:
    void sendOutputReport(const QByteArray& reportData, unsigned int reportID);

  protected:
    void timerEvent(QTimerEvent* event) override;

  private:
    static constexpr int kNumBuffers = 2;
    static constexpr int kBufferSize = 255;
    unsigned char m_pPollData[kNumBuffers][kBufferSize];
    int m_lastPollSize;
    int m_pollingBufferIndex;
    const RuntimeLoggingCategory m_logBase;
    const RuntimeLoggingCategory m_logInput;
    const RuntimeLoggingCategory m_logOutput;

    void poll();
    void processInputReport(int bytesRead);
    hid_device* const
            m_pHidDevice; // const pointer to the C data structure, which hidapi uses for communication between functions
    const mixxx::hid::DeviceInfo m_deviceInfo;
    std::map<unsigned char, std::unique_ptr<HidIoReport>> m_outputReports;

    // Must be locked when using the m_pHidDevice and it's properties, which is not thread-safe for hidapi backends
    QT_RECURSIVE_MUTEX m_HidDeviceMutex;
    int mPollTimerId;
};
