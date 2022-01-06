#pragma once

#include "controllers/controller.h"
#include "controllers/hid/hiddevice.h"
#include "util/duration.h"

class HidIoReport {
  public:
    HidIoReport(const unsigned char& reportId,
            hid_device* device,
            const mixxx::hid::DeviceInfo&& deviceInfo);
    void sendOutputReport(QByteArray data);

  private:
    const unsigned char m_reportId;
    const RuntimeLoggingCategory m_logOutput;
    hid_device* const
            m_pHidDevice; // const pointer to the C data structure, which hidapi uses for communication between functions
    const mixxx::hid::DeviceInfo m_deviceInfo;
    QByteArray m_lastSentOutputReportData;
};
