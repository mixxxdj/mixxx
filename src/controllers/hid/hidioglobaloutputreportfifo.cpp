#include "controllers/hid/hidioglobaloutputreportfifo.h"

#include <hidapi.h>

#include "controllers/defs_controllers.h"
#include "controllers/hid/legacyhidcontrollermappingfilehandler.h"
#include "util/compatibility/qbytearray.h"
#include "util/string.h"
#include "util/time.h"
#include "util/trace.h"

namespace {
constexpr int kReportIdSize = 1;
constexpr int kMaxHidErrorMessageSize = 512;
} // namespace

HidIoGlobalOutputReportFifo::HidIoGlobalOutputReportFifo()
        : m_indexOfLastSentReport(0),
          m_indexOfLastCachedReport(0) {
}

void HidIoGlobalOutputReportFifo::addReportDatasetToFifo(const quint8 reportId,
        const QByteArray& data,
        const mixxx::hid::DeviceInfo& deviceInfo,
        const RuntimeLoggingCategory& logOutput) {
    auto cacheLock = lockMutex(&m_fifoMutex);

    unsigned int indexOfReportToCache;

    if (m_indexOfLastCachedReport + 1 < kSizeOfFifoInReports) {
        indexOfReportToCache = m_indexOfLastCachedReport + 1;
    } else {
        indexOfReportToCache = 0;
    }

    // Handle the case, that the FIFO is full - which is an error case
    if (m_indexOfLastSentReport == indexOfReportToCache) {
        // If the FIFO is full, we skip the report dataset even
        // in non-skipping mode, to keep the controller mapping thread
        // responsive for InputReports from the controller.
        // Alternative would be to block processing of the controller
        // mapping thread, until the FIFO has space again.
        qCWarning(logOutput)
                << "FIFO overflow: Unable to add OutputReport " << reportId
                << "to the global cache for non-skipping sending of OututReports for"
                << deviceInfo.formatName();
        return;
    }

    // First byte must always contain the correct ReportID-Byte,
    // also after swapping
    QByteArray cachedData;
    cachedData.reserve(kReportIdSize + data.size());
    cachedData.append(reportId);
    cachedData.append(data);

    // Swap report data to FIFO
    cachedData.swap(m_outputReportFifo[indexOfReportToCache]);

    m_indexOfLastCachedReport = indexOfReportToCache;
}

bool HidIoGlobalOutputReportFifo::sendNextReportDataset(QMutex* pHidDeviceAndPollMutex,
        hid_device* pHidDevice,
        const mixxx::hid::DeviceInfo& deviceInfo,
        const RuntimeLoggingCategory& logOutput) {
    auto startOfHidWrite = mixxx::Time::elapsed();

    auto fifoLock = lockMutex(&m_fifoMutex);

    if (m_indexOfLastSentReport == m_indexOfLastCachedReport) {
        // No data in FIFO to be send
        // Return with false, to signal the caller, that no time consuming IO
        // operation was ncessary
        return false;
    }

    // Store old values for use in controller debug output after fifoLock.unlock()
    unsigned int indexOfLastCachedReport = m_indexOfLastCachedReport;
    unsigned int indexOfLastSentReport = m_indexOfLastSentReport;

    // Preemptively set m_indexOfLastSentReport and swap
    // m_outputReportFifo[m_indexOfLastSentReport], to release the fifoLock
    // mutex before the time consuming hid_write operation.
    if (m_indexOfLastSentReport + 1 < kSizeOfFifoInReports) {
        m_indexOfLastSentReport++;
    } else {
        m_indexOfLastSentReport = 0;
    }

    QByteArray dataToSend;
    dataToSend.swap(m_outputReportFifo[m_indexOfLastSentReport]);

    fifoLock.unlock();

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
                           << ") sent from non-skipping FIFO ("
                           << (indexOfLastCachedReport > indexOfLastSentReport
                                              ? indexOfLastCachedReport -
                                                      indexOfLastSentReport
                                              : kSizeOfFifoInReports -
                                                      indexOfLastSentReport +
                                                      indexOfLastCachedReport)
                           << "/" << kSizeOfFifoInReports << "used) - Needed: "
                           << (mixxx::Time::elapsed() - startOfHidWrite)
                                      .formatMicrosWithUnit();
    }

    // Return with true, to signal the caller, that the time consuming hid_write
    // operation was executed
    return true;
}
