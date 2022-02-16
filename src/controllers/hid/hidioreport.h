#pragma once

#include "controllers/controller.h"
#include "controllers/hid/hiddevice.h"
#include "util/compatibility/qmutex.h"
#include "util/duration.h"

class HidIoReport {
  public:
    HidIoReport(const unsigned char& reportId, const unsigned int& reportDataSize);

    /// Caches new report data, which will later send by the IO thread
    void cacheOutputReport(const QByteArray& data,
            const mixxx::hid::DeviceInfo& deviceInfo,
            const RuntimeLoggingCategory& logOutput);

    /// Sends the OutputReport to the HID device, when changed data are cached.
    /// Returns true if a time consuming hid_write operation was executed.
    bool sendOutputReport(QMutex* pHidDeviceMutex,
            hid_device* pHidDevice,
            const mixxx::hid::DeviceInfo& deviceInfo,
            const RuntimeLoggingCategory& logOutput);

  private:
    const unsigned char m_reportId;
    QByteArray m_lastSentOutputReportData;

    /// Mutex must be locked when reading/writing m_cachedOutputReportData
    /// or m_possiblyUnsentDataCached
    QMutex m_cachedOutputReportDataMutex;

    QByteArray m_cachedOutputReportData;
    bool m_possiblyUnsentDataCached;

    /// Due to swapping of the QbyteArrays, we need to store
    /// this information independent of the QBytearray size
    int m_lastCachedDataSize;
};
