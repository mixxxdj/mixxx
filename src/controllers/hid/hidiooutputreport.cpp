#include "controllers/hid/hidiooutputreport.h"

#include <hidapi.h>

#include "util/cmdlineargs.h"
#include "util/runtimeloggingcategory.h"
#include "util/string.h"
#include "util/time.h"

namespace {
constexpr int kReportIdSize = 1;
constexpr size_t kMaxHidErrorMessageSize = 512;
} // namespace

HidIoOutputReport::HidIoOutputReport(
        const quint8& reportId, const unsigned int& reportDataSize)
        : m_reportId(reportId),
          m_hidWriteErrorLogged(false),
          m_possiblyUnsentDataCached(false),
          m_lastCachedDataSize(0) {
    // First byte must always contain the ReportID - also after swapping, therefore initialize both arrays
    m_cachedData.reserve(kReportIdSize + reportDataSize);
    m_cachedData.append(reportId);
    m_lastSentData.reserve(kReportIdSize + reportDataSize);
    m_lastSentData.append(reportId);
}

void HidIoOutputReport::updateCachedData(const QByteArray& data,
        const RuntimeLoggingCategory& logOutput,
        bool useNonSkippingFIFO) {
    auto cacheLock = lockMutex(&m_cachedDataMutex);

    if (!m_lastCachedDataSize) {
        // First call updateCachedData for this report
        m_lastCachedDataSize = data.size();

    } else {
        if (CmdlineArgs::Instance()
                        .getControllerDebug() &&
                m_possiblyUnsentDataCached && !useNonSkippingFIFO) {
            qCDebug(logOutput) << "t:" << mixxx::Time::elapsed().formatMillisWithUnit()
                               << "skipped superseded OutputReport data for ReportID"
                               << m_reportId;
        }

        // The size of an HID report is defined in a HID device and can't vary at runtime
        if (m_lastCachedDataSize != data.size()) {
            qCWarning(logOutput)
                    << "Size of OutputReport ( with ReportID" << m_reportId
                    << ") changed from" << m_lastCachedDataSize << "to"
                    << data.size()
                    << "- This indicates a bug in the mapping code!";
            m_lastCachedDataSize = data.size();
        }
    }

    // m_possiblyUnsentDataCached must be set while m_cachedDataMutex is locked
    // This step covers the case that data for the report are cached in skipping mode,
    // succeed by a non-skipping send of the same report
    if (useNonSkippingFIFO) {
        m_possiblyUnsentDataCached = false;
        m_lastSentData.clear();
        return;
    }

    // Deep copy with reusing the already allocated heap memory
    // The first byte with the ReportID is not overwritten
    m_cachedData.replace(
            kReportIdSize,
            m_cachedData.size(),
            data.constData(),
            data.size());
    m_possiblyUnsentDataCached = true;
}

bool HidIoOutputReport::sendCachedData(QMutex* pHidDeviceAndPollMutex,
        hid_device* pHidDevice,
        const RuntimeLoggingCategory& logOutput) {
    auto startOfHidWrite = mixxx::Time::elapsed();

    auto cacheLock = lockMutex(&m_cachedDataMutex);

    if (!m_possiblyUnsentDataCached) {
        // Return with false, to signal the caller, that no time consuming IO operation was necessary
        return false;
    }

    if (!m_lastSentData.compare(m_cachedData)) {
        // An HID OutputReport can contain only HID OutputItems.
        // HID OutputItems are defined to represent the state of one or more similar controls or LEDs.
        // Only HID Feature items may be attributes of other items.
        // This means there is always a one to one relationship to the state of control(s)/LED(s),
        // and if the state is not changed, there's no need to execute the time consuming hid_write again.

        // Setting m_possiblyUnsentDataCached to false prevents,
        // that the byte array compare operation is executed for the same data again
        m_possiblyUnsentDataCached = false;

        cacheLock.unlock();

        if (CmdlineArgs::Instance()
                        .getControllerDebug()) {
            qCDebug(logOutput) << "t:" << startOfHidWrite.formatMillisWithUnit()
                               << "Skipped sending identical OutputReport data "
                                  "from cache for ReportID"
                               << m_reportId;
        }

        // Return with false, to signal the caller, that no time consuming IO operation was necessary
        return false;
    }

    // Preemptively set m_lastSentData and m_possiblyUnsentDataCached,
    // to release the mutex during the time consuming hid_write operation.
    // In the unlikely case that hid_write fails, they will be invalidated afterwards
    // This is safe, because these members are only reset in this scope of this method,
    // and concurrent execution of this method is prevented by locking pHidDeviceMutex
    m_lastSentData.swap(m_cachedData);
    m_possiblyUnsentDataCached = false;

    cacheLock.unlock();

    auto hidDeviceLock = lockMutex(pHidDeviceAndPollMutex);

    // hid_write can take several milliseconds, because hidapi synchronizes
    // the asyncron HID communication from the OS
    int result = hid_write(pHidDevice,
            reinterpret_cast<const unsigned char*>(m_lastSentData.constData()),
            m_lastSentData.size());
    if (result == -1) {
        if (!m_hidWriteErrorLogged) {
            qCWarning(logOutput)
                    << "Unable to send data to device :"
                    << mixxx::convertWCStringToQString(
                               hid_error(pHidDevice), kMaxHidErrorMessageSize)
                    << "Note that, this message is only logged once and may "
                       "not appear again until all hid_writee errors have "
                       "disappeared.";

            // Stop logging error messages if every hid_write() fails to avoid large log files
            m_hidWriteErrorLogged = true;
        }
    } else {
        m_hidWriteErrorLogged = false;
    }

    hidDeviceLock.unlock();

    if (result == -1) {
        cacheLock.relock();
        // Clear the m_lastSentData because the last send data are not reliable known.
        // These error should not occur in normal operation,
        // therefore the performance impact of additional memory allocation
        // at the next call of this method is negligible
        m_lastSentData.clear();
        m_lastSentData.append(m_reportId);
        m_possiblyUnsentDataCached = true;

        // Return with true, to signal the caller, that the time consuming hid_write operation was executed
        // (Note, that the return value isn't an error code)
        return true;
    }

    if (CmdlineArgs::Instance()
                    .getControllerDebug()) {
        qCDebug(logOutput) << "t:" << startOfHidWrite.formatMillisWithUnit() << " "
                           << result << "bytes ( including ReportID of"
                           << static_cast<quint8>(m_reportId)
                           << ") sent from skipping cache - Needed:"
                           << (mixxx::Time::elapsed() - startOfHidWrite).formatMicrosWithUnit();
    }

    // Return with true, to signal the caller, that the time consuming hid_write operation was executed
    return true;
}
