#pragma once

#include "controllers/controller.h"
#include "controllers/hid/hiddevice.h"
#include "controllers/hid/hidioglobaloutputreportfifo.h"
#include "util/compatibility/qmutex.h"
#include "util/duration.h"

class HidIoOutputReport {
  public:
    HidIoOutputReport(const quint8& reportId, const unsigned int& reportDataSize);

    /// Caches new report data, which will later send by the IO thread
    void updateCachedData(const QByteArray& data,
            const mixxx::hid::DeviceInfo& deviceInfo,
            const RuntimeLoggingCategory& logOutput,
            HidIoGlobalOutputReportFifo* pGlobalOutputReportFifo,
            bool useNonSkippingFIFO);

    /// Sends the OutputReport to the HID device, when changed data are cached.
    /// Returns true if a time consuming hid_write operation was executed.
    bool sendCachedData(QMutex* pHidDeviceAndPollMutex,
            hid_device* pHidDevice,
            const mixxx::hid::DeviceInfo& deviceInfo,
            const RuntimeLoggingCategory& logOutput);

  private:
    const quint8 m_reportId;
    QByteArray m_lastSentData;

    /// Mutex must be locked when reading/writing m_cachedData
    /// or m_possiblyUnsentDataCached
    QMutex m_cachedDataMutex;

    QByteArray m_cachedData;
    bool m_possiblyUnsentDataCached;

    /// Due to swapping of the QbyteArrays, we need to store
    /// this information independent of the QBytearray size
    int m_lastCachedDataSize;
};
