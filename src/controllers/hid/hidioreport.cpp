#include "controllers/hid/hidioreport.h"

#include <hidapi.h>

#include "controllers/defs_controllers.h"
#include "controllers/hid/legacyhidcontrollermappingfilehandler.h"
#include "util/string.h"
#include "util/time.h"
#include "util/trace.h"

namespace {
constexpr int kReportIdSize = 1;
constexpr int kMaxHidErrorMessageSize = 512;
} // namespace

HidIoReport::HidIoReport(const unsigned char& reportId)
        : m_reportId(reportId),
          m_possiblyUnsendDataCached(false) {
}

void HidIoReport::cacheOutputReport(const QByteArray& data,
        const mixxx::hid::DeviceInfo& deviceInfo,
        const RuntimeLoggingCategory& logOutput) {
    auto lock = lockMutex(&m_cachedOutputReportDataMutex);
    if (m_possiblyUnsendDataCached) {
        qCDebug(logOutput) << "t:" << mixxx::Time::elapsed().formatMillisWithUnit()
                           << " Skipped superseded OutputReport"
                           << deviceInfo.formatName() << "serial #"
                           << deviceInfo.serialNumberRaw() << "(Report ID"
                           << m_reportId << ")";
    }
    // Deep copy with reusing the already allocated heap memory
    m_cachedOutputReportData.replace(0, m_cachedOutputReportData.size(), data, data.size());
    m_possiblyUnsendDataCached = true;
}

bool HidIoReport::sendOutputReport(QMutex* pHidDeviceMutex,
        hid_device* pHidDevice,
        const mixxx::hid::DeviceInfo& deviceInfo,
        const RuntimeLoggingCategory& logOutput) {
    auto startOfHidWrite = mixxx::Time::elapsed();
    QByteArray reportToSend;

    {
        auto lock = lockMutex(&m_cachedOutputReportDataMutex);

        if (!m_possiblyUnsendDataCached) {
            // Return with false, to signal the caller, that no time consuming IO operation was necessary
            return false;
        }

        if (!m_lastSentOutputReportData.compare(m_cachedOutputReportData)) {
            // An HID OutputReport can contain only HID OutputItems.
            // HID OutputItems are defined to represent the state of one or more similar controls or LEDs.
            // Only HID Feature items may be attributes of other items.
            // This means there is always a one to one relationship to the state of control(s)/LED(s),
            // and if the state is not changed, there's no need to execute the time consuming hid_write again.

            // Setting m_possiblyUnsendDataCached to false prevents,
            // that the byte array compare operation is executed for the same data again
            m_possiblyUnsendDataCached = false;

            lock.unlock();

            qCDebug(logOutput) << "t:" << startOfHidWrite.formatMillisWithUnit()
                               << " Skipped identical Output Report for"
                               << deviceInfo.formatName() << "serial #"
                               << deviceInfo.serialNumberRaw() << "(Report ID"
                               << m_reportId << ")";

            // Return with false, to signal the caller, that no time consuming IO operation was necessary
            return false;
        }

        // hid_write requires the first byte to be the Report ID, followed by the data[] to be send
        reportToSend.reserve(m_cachedOutputReportData.size() + kReportIdSize);
        reportToSend.append(m_reportId);
        reportToSend.append(m_cachedOutputReportData);

        // Preemtive set m_lastSentOutputReportData and m_possiblyUnsendDataCached,
        // to release the mutex during the time consuming hid_write operation.
        // In the unlikely case that hid_write fails, they will be invalidated afterwards
        // This is safe, because these members are only reset in this scope of this method,
        // and concurrent execution of this method is prevented by locking pHidDeviceMutex
        m_lastSentOutputReportData.swap(m_cachedOutputReportData);
        m_possiblyUnsendDataCached = false;
    }

    int result;
    {
        auto lock = lockMutex(pHidDeviceMutex);
        // hid_write can take several milliseconds, because hidapi synchronizes the asyncron HID communication from the OS
        result = hid_write(pHidDevice,
                reinterpret_cast<const unsigned char*>(reportToSend.constData()),
                reportToSend.size());
        if (result == -1) {
            qCWarning(logOutput) << "Unable to send data to" << deviceInfo.formatName() << ":"
                                 << mixxx::convertWCStringToQString(
                                            hid_error(pHidDevice),
                                            kMaxHidErrorMessageSize);
        }
    }

    if (result == -1) {
        auto lock = lockMutex(&m_cachedOutputReportDataMutex);
        // Clear the m_lastSentOutputReportData because the last send data are not reliable known.
        // These error should not occur in normal operation,
        // therefore the performance impact of additional memory allocation
        // at the next call of this method is negligible
        m_lastSentOutputReportData.clear();
        m_possiblyUnsendDataCached = true;

        // Return with true, to signal the caller, that the time consuming hid_write operation was executed
        // (Note, that the return value isn't an error code)
        return true;
    }

    qCDebug(logOutput) << "t:" << startOfHidWrite.formatMillisWithUnit() << " "
                       << result << "bytes sent to" << deviceInfo.formatName()
                       << "serial #" << deviceInfo.serialNumberRaw()
                       << "(including report ID of" << m_reportId << ") - Needed: "
                       << (mixxx::Time::elapsed() - startOfHidWrite).formatMicrosWithUnit();

    // Return with true, to signal the caller, that the time consuming hid_write operation was executed
    return true;
}
