#include "controllers/hid/hidioglobaloutputreportfifo.h"

#include <hidapi.h>

#include "controllers/defs_controllers.h"
#include "controllers/hid/legacyhidcontrollermappingfilehandler.h"
#include "util/compatibility/qbytearray.h"
#include "util/compatibility/qmutex.h"
#include "util/string.h"
#include "util/time.h"
#include "util/trace.h"

namespace {
constexpr size_t kMaxHidErrorMessageSize = 512;
constexpr size_t kSizeOfFifoInReports = 32;
} // namespace

HidIoGlobalOutputReportFifo::HidIoGlobalOutputReportFifo()
        : m_fifoQueue(kSizeOfFifoInReports) {
}

void HidIoGlobalOutputReportFifo::addReportDatasetToFifo(const quint8 reportId,
        const QByteArray& reportData,
        const mixxx::hid::DeviceInfo& deviceInfo,
        const RuntimeLoggingCategory& logOutput) {
    // First byte must always be the ReportID-Byte
    QByteArray report(reportData);
    report.prepend(reportId); // In Qt6 this is a very fast operation without reallocation

    // Swap report to lockless FIFO queue
    bool success = m_fifoQueue.try_emplace(std::move(report));

    // Handle the case, that the FIFO queue is full - which is an error case
    if (!success) {
        // If the FIFO is full, we skip the report dataset even
        // in non-skipping mode, to keep the controller mapping thread
        // responsive for InputReports from the controller.
        // Alternative would be to block processing of the controller
        // mapping thread, until the FIFO has space again.
        qCWarning(logOutput)
                << "FIFO overflow: Unable to add OutputReport " << reportId
                << "to the global cache for non-skipping sending of OututReports for"
                << deviceInfo.formatName();
    }
}

bool HidIoGlobalOutputReportFifo::sendNextReportDataset(QMutex* pHidDeviceAndPollMutex,
        hid_device* pHidDevice,
        const mixxx::hid::DeviceInfo& deviceInfo,
        const RuntimeLoggingCategory& logOutput) {
    auto startOfHidWrite = mixxx::Time::elapsed();

    QByteArray* pFront = m_fifoQueue.front();

    if (pFront == nullptr) {
        // No data in FIFO to be send
        // Return with false, to signal the caller, that no time consuming IO
        // operation was ncessary
        return false;
    }

    QByteArray dataToSend(std::move(*pFront));
    m_fifoQueue.pop();

    auto hidDeviceLock = lockMutex(pHidDeviceAndPollMutex);

    // hid_write can take several milliseconds, because hidapi synchronizes
    // the asyncron HID communication from the OS
    int result = hid_write(pHidDevice,
            reinterpret_cast<const unsigned char*>(dataToSend.constData()),
            dataToSend.size());
    if (result == -1) {
        qCWarning(logOutput) << "Unable to send data to" << deviceInfo.formatName() << ":"
                             << mixxx::convertWCStringToQString(
                                        hid_error(pHidDevice),
                                        kMaxHidErrorMessageSize);
    }

    hidDeviceLock.unlock();

    if (result != -1) {
        qCDebug(logOutput) << "t:" << startOfHidWrite.formatMillisWithUnit()
                           << " " << result << "bytes (including ReportID of"
                           << static_cast<quint8>(dataToSend[0])
                           << ") sent from non-skipping FIFO - Needed: "
                           << (mixxx::Time::elapsed() - startOfHidWrite)
                                      .formatMicrosWithUnit();
    }

    // Return with true, to signal the caller, that the time consuming hid_write
    // operation was executed
    return true;
}
