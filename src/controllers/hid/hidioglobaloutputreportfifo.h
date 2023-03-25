#pragma once

#include "controllers/controller.h"
#include "controllers/hid/hiddevice.h"
#include "rigtorp/SPSCQueue.h"
#include "util/duration.h"

/// Stores and sends OutputReports (independent of the ReportID) in First In /
/// First Out (FIFO) order
class HidIoGlobalOutputReportFifo {
  public:
    HidIoGlobalOutputReportFifo();

    /// Caches new OutputReport to the FIFO, which will later be send by the IO thread
    void addReportDatasetToFifo(const quint8 reportId,
            const QByteArray& reportData,
            const mixxx::hid::DeviceInfo& deviceInfo,
            const RuntimeLoggingCategory& logOutput);

    /// Sends the next OutputReport from FIFO to the HID device,
    /// when if any report is cached in FIFO.
    /// Returns true if a time consuming hid_write operation was executed.
    bool sendNextReportDataset(QMutex* pHidDeviceAndPollMutex,
            hid_device* pHidDevice,
            const mixxx::hid::DeviceInfo& deviceInfo,
            const RuntimeLoggingCategory& logOutput);

  private:
    // Lockless FIFO queue
    rigtorp::SPSCQueue<QByteArray> m_fifoQueue;
};
