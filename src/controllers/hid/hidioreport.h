#pragma once

#include "controllers/controller.h"
#include "controllers/hid/hiddevice.h"
#include "util/compatibility/qmutex.h"
#include "util/duration.h"

class HidIoReport {
  public:
    HidIoReport(const unsigned char& reportId,
            hid_device* device);
    void latchOutputReport(const QByteArray& data,
            const mixxx::hid::DeviceInfo& deviceInfo,
            const RuntimeLoggingCategory& logOutput);
    bool sendOutputReport(
            const mixxx::hid::DeviceInfo& deviceInfo,
            const RuntimeLoggingCategory& logOutput);

  private:
    const unsigned char m_reportId;
    hid_device* const
            m_pHidDevice; // const pointer to the C data structure, which hidapi uses for communication between functions
    QByteArray m_lastSentOutputReportData;
    QByteArray m_latchedOutputReportData;

    // Must be locked when reading/writing m_lastSentOutputReportData and m_latchedOutputReportData
    QT_RECURSIVE_MUTEX m_OutputReportDataMutex;
};
