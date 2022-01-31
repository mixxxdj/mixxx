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
          m_unsendDataLatched(false) {
}

void HidIoReport::latchOutputReport(const QByteArray& data,
        const mixxx::hid::DeviceInfo& deviceInfo,
        const RuntimeLoggingCategory& logOutput) {
    auto lock = lockMutex(&m_outputReportDataMutex);
    if (m_unsendDataLatched) {
        qCDebug(logOutput) << "t:" << mixxx::Time::elapsed().formatMillisWithUnit()
                           << " Skipped superseded OutputReport"
                           << deviceInfo.formatName() << "serial #"
                           << deviceInfo.serialNumberRaw() << "(Report ID"
                           << m_reportId << ")";
    }
    m_latchedOutputReportData.replace(0, data.size(), data);
    m_unsendDataLatched = true;
}

bool HidIoReport::sendOutputReport(hid_device* pHidDevice,
        const mixxx::hid::DeviceInfo& deviceInfo,
        const RuntimeLoggingCategory& logOutput) {
    auto startOfHidWrite = mixxx::Time::elapsed();

    auto lock = lockMutex(&m_outputReportDataMutex);

    if (m_unsendDataLatched == false) {
        return false; // Return with false, to signal the caller, that no time consuming IO operation was necessary
    }

    if (!m_lastSentOutputReportData.compare(m_latchedOutputReportData)) {
        // An HID OutputReport can contain only HID OutputItems.
        // HID OutputItems are defined to represent the state of one or more similar controls or LEDs.
        // Only HID Feature items may be attributes of other items.
        // This means there is always a one to one relationship to the state of control(s)/LED(s). And if the state is not changed, there's no need to execute the time consuming hid_write again.
        qCDebug(logOutput) << "t:" << startOfHidWrite.formatMillisWithUnit()
                           << " Skipped identical Output Report for"
                           << deviceInfo.formatName() << "serial #"
                           << deviceInfo.serialNumberRaw() << "(Report ID"
                           << m_reportId << ")";
        m_unsendDataLatched =
                false; // Setting m_unsendDataLatched to false prevents, that the byte array compare operation is executed for the same data again
        return false; // Return with false, to signal the caller, that no time consuming IO operation was necessary
    }

    QByteArray reportToSend;
    // hid_write requires the first byte to be the Report ID, followed by the data[] to be send
    reportToSend.reserve(m_latchedOutputReportData.size() + kReportIdSize);
    reportToSend.append(m_reportId);
    reportToSend.append(m_latchedOutputReportData);

    // hid_write can take several milliseconds, because hidapi synchronizes the asyncron HID communication from the OS
    int result = hid_write(pHidDevice,
            reinterpret_cast<const unsigned char*>(reportToSend.constData()),
            kReportIdSize + m_latchedOutputReportData.size());
    if (result == -1) {
        qCWarning(logOutput) << "Unable to send data to" << deviceInfo.formatName() << ":"
                             << mixxx::convertWCStringToQString(
                                        hid_error(pHidDevice),
                                        kMaxHidErrorMessageSize);
        return true; // Return with true, to signal the caller, that the time consuming hid_write operation was executed - (Note that the return value isn't an error code)
    }

    qCDebug(logOutput) << "t:" << startOfHidWrite.formatMillisWithUnit() << " "
                       << result << "bytes sent to" << deviceInfo.formatName()
                       << "serial #" << deviceInfo.serialNumberRaw()
                       << "(including report ID of" << m_reportId << ") - Needed: "
                       << (mixxx::Time::elapsed() - startOfHidWrite).formatMicrosWithUnit();

    m_lastSentOutputReportData.swap(m_latchedOutputReportData);
    m_unsendDataLatched = false;
    return true; // Return with true, to signal the caller, that the time consuming hid_write operation was executed
}
