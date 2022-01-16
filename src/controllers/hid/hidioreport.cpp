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
        hid_device* device,
        std::shared_ptr<const mixxx::hid::DeviceInfo> deviceInfo)
        : m_reportId(reportId),
          m_logOutput(loggingCategoryPrefix(deviceInfo->formatName()) + QStringLiteral(".output")),
          m_pHidDevice(device),
          m_pDeviceInfo(deviceInfo) {
}

void HidIoReport::sendOutputReport(QByteArray data) {
    auto startOfHidWrite = mixxx::Time::elapsed();
    if (!m_lastSentOutputReportData.compare(data)) {
        qCDebug(m_logOutput) << "t:" << startOfHidWrite.formatMillisWithUnit()
                             << " Skipped identical Output Report for"
                             << m_pDeviceInfo->formatName() << "serial #"
                             << m_pDeviceInfo->serialNumberRaw() << "(Report ID"
                             << m_reportId << ")";
        return; // Same data sent last time
    }

    // hid_write requires the first byte to be the Report ID, followed by the data[] to be send
    QByteArray outputReport;
    outputReport.reserve(data.size() + kReportIdSize);
    outputReport.append(m_reportId);
    outputReport.append(data);

    // hid_write can take several milliseconds, because hidapi synchronizes the asyncron HID communication from the OS
    int result = hid_write(m_pHidDevice,
            reinterpret_cast<const unsigned char*>(outputReport.constData()),
            outputReport.size());
    if (result == -1) {
        qCWarning(m_logOutput) << "Unable to send data to" << m_pDeviceInfo->formatName() << ":"
                               << mixxx::convertWCStringToQString(
                                          hid_error(m_pHidDevice),
                                          kMaxHidErrorMessageSize);
        return;
    }

    qCDebug(m_logOutput) << "t:" << startOfHidWrite.formatMillisWithUnit() << " "
                         << result << "bytes sent to" << m_pDeviceInfo->formatName()
                         << "serial #" << m_pDeviceInfo->serialNumberRaw()
                         << "(including report ID of" << m_reportId << ") - Needed: "
                         << (mixxx::Time::elapsed() - startOfHidWrite).formatMicrosWithUnit();

    m_lastSentOutputReportData = std::move(data);
}
