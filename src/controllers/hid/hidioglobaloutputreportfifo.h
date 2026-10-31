#pragma once

#include <QByteArray>

#include "rigtorp/SPSCQueue.h"

struct RuntimeLoggingCategory;
class QMutex;
struct hid_device_;
typedef struct hid_device_ hid_device;

namespace mixxx {
namespace hid {
class DeviceInfo;
} // namespace hid
} // namespace mixxx

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
    bool m_hidWriteErrorLogged;
};
