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
QString loggingCategoryPrefix(const QString& deviceName) {
    return QStringLiteral("controller.") +
            RuntimeLoggingCategory::removeInvalidCharsFromCategory(deviceName.toLower());
}
} // namespace

HidIoReport::HidIoReport(const unsigned char& reportId,
        hid_device* pDevice,
        std::shared_ptr<const mixxx::hid::DeviceInfo> pDeviceInfo)
        : m_reportId(reportId),
          m_logOutput(loggingCategoryPrefix(pDeviceInfo->formatName()) + QStringLiteral(".output")),
          m_pHidDevice(pDevice),
          m_pDeviceInfo(pDeviceInfo) {
}

void HidIoReport::latchOutputReport(QByteArray data) {
    auto lock = lockMutex(&m_OutputReportDataMutex);
    if (!m_latchedOutputReportData.isEmpty()) {
        qCDebug(m_logOutput) << "t:" << mixxx::Time::elapsed().formatMillisWithUnit()
                             << " Skipped superseded OutputReport"
                             << m_pDeviceInfo->formatName() << "serial #"
                             << m_pDeviceInfo->serialNumberRaw() << "(Report ID"
                             << m_reportId << ")";
    }
    m_latchedOutputReportData.clear();
    // hid_write requires the first byte to be the Report ID, followed by the data[] to be send
    m_latchedOutputReportData.reserve(data.size() + kReportIdSize);
    m_latchedOutputReportData.append(m_reportId);
    m_latchedOutputReportData.append(data);
}

bool HidIoReport::sendOutputReport() {
    auto startOfHidWrite = mixxx::Time::elapsed();

    auto lock = lockMutex(&m_OutputReportDataMutex);

    if (m_latchedOutputReportData.isEmpty()) {
        return false;
    }

    if (!m_lastSentOutputReportData.compare(m_latchedOutputReportData)) {
        // An HID OutputReport can contain only HID OutputItems.
        // HID OutputItems are defined to represent the state of one or more similar controls or LEDs.
        // Only HID Feature items may be attributes of other items.
        // This means there is always a one to one relationship to the state of control(s)/LED(s). And if the state is not changed, there's no need to execute the time consuming hid_write again.
        qCDebug(m_logOutput) << "t:" << startOfHidWrite.formatMillisWithUnit()
                             << " Skipped identical Output Report for"
                             << m_pDeviceInfo->formatName() << "serial #"
                             << m_pDeviceInfo->serialNumberRaw() << "(Report ID"
                             << m_reportId << ")";
        m_latchedOutputReportData.clear();
        return false; // Same data sent last time
    }

    // hid_write can take several milliseconds, because hidapi synchronizes the asyncron HID communication from the OS
    int result = hid_write(m_pHidDevice,
            reinterpret_cast<const unsigned char*>(m_latchedOutputReportData.constData()),
            m_latchedOutputReportData.size());
    if (result == -1) {
        qCWarning(m_logOutput) << "Unable to send data to" << m_pDeviceInfo->formatName() << ":"
                               << mixxx::convertWCStringToQString(
                                          hid_error(m_pHidDevice),
                                          kMaxHidErrorMessageSize);
        return false;
    }

    qCDebug(m_logOutput) << "t:" << startOfHidWrite.formatMillisWithUnit() << " "
                         << result << "bytes sent to" << m_pDeviceInfo->formatName()
                         << "serial #" << m_pDeviceInfo->serialNumberRaw()
                         << "(including report ID of" << m_reportId << ") - Needed: "
                         << (mixxx::Time::elapsed() - startOfHidWrite).formatMicrosWithUnit();

    m_lastSentOutputReportData = std::move(m_latchedOutputReportData);
    m_latchedOutputReportData.clear();
    return true;
}