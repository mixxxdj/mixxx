#pragma once

#include "controllers/controller.h"
#include "controllers/hid/hiddevice.h"
#include "util/compatibility/qmutex.h"
#include "util/duration.h"

class HidIoReport {
  public:
    HidIoReport(const unsigned char& reportId);

    /// Latches new report data, which will later send by the IO thread
    void latchOutputReport(const QByteArray& data,
            const mixxx::hid::DeviceInfo& deviceInfo,
            const RuntimeLoggingCategory& logOutput);

    /// Sends the OutputReport to the HID device, when changed data are latched.
    /// Returns true if a time consuming hid_write operation was executed.
    bool sendOutputReport(hid_device* pDevice,
            const mixxx::hid::DeviceInfo& deviceInfo,
            const RuntimeLoggingCategory& logOutput);

  private:
    const unsigned char m_reportId;
    QByteArray m_lastSentOutputReportData;
    QByteArray m_latchedOutputReportData;
    bool m_unsendDataLatched;

    /// Must be locked when reading/writing m_lastSentOutputReportData, m_latchedOutputReportData or m_unsendDataLatched
    QMutex m_outputReportDataMutex;
};
