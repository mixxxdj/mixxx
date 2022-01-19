#pragma once

#include "controllers/controller.h"
#include "controllers/hid/hiddevice.h"
#include "util/compatibility/qmutex.h"
#include "util/duration.h"

class HidIoReport {
  public:
    HidIoReport(const unsigned char& reportId,
            hid_device* device,
            std::shared_ptr<const mixxx::hid::DeviceInfo> deviceInfo);
    void latchOutputReport(const QByteArray& data);
    bool sendOutputReport();

  private:
    const unsigned char m_reportId;
    const RuntimeLoggingCategory m_logOutput;
    hid_device* const
            m_pHidDevice; // const pointer to the C data structure, which hidapi uses for communication between functions
    std::shared_ptr<const mixxx::hid::DeviceInfo> m_pDeviceInfo;
    QByteArray m_lastSentOutputReportData;
    QByteArray m_latchedOutputReportData;

    // Must be locked when reading/writing m_lastSentOutputReportData and m_latchedOutputReportData
    QT_RECURSIVE_MUTEX m_OutputReportDataMutex;
};
