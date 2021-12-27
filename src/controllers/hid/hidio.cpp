#include "controllers/hid/hidio.h"

#include <hidapi.h>

#include "controllers/defs_controllers.h"
#include "controllers/hid/legacyhidcontrollermappingfilehandler.h"
#include "moc_hidio.cpp"
#include "util/string.h"
#include "util/time.h"
#include "util/trace.h"

namespace {
constexpr int kReportIdSize = 1;
constexpr int kMaxHidErrorMessageSize = 512;
} // namespace

HidIoReport::HidIoReport(const unsigned char& reportId,
        hid_device* device,
        const mixxx::hid::DeviceInfo&& deviceInfo,
        const RuntimeLoggingCategory& logOutput)
        : m_reportId(reportId),
          m_pHidDevice(device),
          m_deviceInfo(std::move(deviceInfo)),
          m_logOutput(logOutput),
          m_lastSentOutputreport("") {
}

HidIoReport::~HidIoReport() {
}

void HidIoReport::sendOutputReport(QByteArray data) {
    auto startOfHidWrite = mixxx::Time::elapsed();
    if (!m_lastSentOutputreport.compare(data)) {
        qCDebug(m_logOutput) << "t:" << startOfHidWrite.formatMillisWithUnit()
                             << " Skipped identical Output Report for" << m_deviceInfo.formatName()
                             << "serial #" << m_deviceInfo.serialNumberRaw()
                             << "(Report ID" << m_reportId << ")";
        return; // Same data sent last time
    }
    m_lastSentOutputreport.clear();
    m_lastSentOutputreport.append(data);

    // Prepend the Report ID to the beginning of data[] per the API..
    data.prepend(m_reportId);

    // hid_write can take several milliseconds, because hidapi synchronizes the asyncron HID communication from the OS
    int result = hid_write(m_pHidDevice, (unsigned char*)data.constData(), data.size());
    if (result == -1) {
        qCWarning(m_logOutput) << "Unable to send data to" << m_deviceInfo.formatName() << ":"
                               << mixxx::convertWCStringToQString(
                                          hid_error(m_pHidDevice),
                                          kMaxHidErrorMessageSize);
    } else {
        qCDebug(m_logOutput) << "t:" << startOfHidWrite.formatMillisWithUnit() << " "
                             << result << "bytes sent to" << m_deviceInfo.formatName()
                             << "serial #" << m_deviceInfo.serialNumberRaw()
                             << "(including report ID of" << m_reportId << ") - Needed: "
                             << (mixxx::Time::elapsed() - startOfHidWrite).formatMicrosWithUnit();
    }
}

HidIo::HidIo(hid_device* device,
        const mixxx::hid::DeviceInfo&& deviceInfo,
        const RuntimeLoggingCategory& logBase,
        const RuntimeLoggingCategory& logInput,
        const RuntimeLoggingCategory& logOutput)
        : QThread(),
          m_pollingBufferIndex(0),
          m_logBase(logBase),
          m_logInput(logInput),
          m_logOutput(logOutput),
          m_pHidDevice(device),
          m_deviceInfo(std::move(deviceInfo)) {
    // This isn't strictly necessary but is good practice.
    for (int i = 0; i < kNumBuffers; i++) {
        memset(m_pPollData[i], 0, kBufferSize);
    }
    m_lastPollSize = 0;
}

HidIo::~HidIo() {
}

void HidIo::run() {
    m_stop = 0;
    while (atomicLoadRelaxed(m_stop) == 0) {
        poll();
        usleep(1000);
    }
}

void HidIo::poll() {
    Trace hidRead("HidIo poll");

    // This loop risks becoming a high priority endless loop in case processing
    // the mapping JS code takes longer than the controller polling rate.
    // This could stall other low priority tasks.
    // There is no safety net for this because it has not been demonstrated to be
    // a problem in practice.
    while (true) {
        int bytesRead = hid_read(m_pHidDevice, m_pPollData[m_pollingBufferIndex], kBufferSize);
        if (bytesRead < 0) {
            // -1 is the only error value according to hidapi documentation.
            DEBUG_ASSERT(bytesRead == -1);
            break;
        } else if (bytesRead == 0) {
            // No packet was available to be read
            break;
        }
        processInputReport(bytesRead);
    }
}

void HidIo::processInputReport(int bytesRead) {
    Trace process("HidIO processInputReport");
    unsigned char* pPreviousBuffer = m_pPollData[(m_pollingBufferIndex + 1) % kNumBuffers];
    unsigned char* pCurrentBuffer = m_pPollData[m_pollingBufferIndex];
    // Some controllers such as the Gemini GMX continuously send input reports even if it
    // is identical to the previous send input report. If this loop processed all those redundant
    // input report, it would be a big performance problem to run JS code for every  input report and
    // would be unnecessary.
    // This assumes that the redundant input report all use the same report ID. In practice we
    // have not encountered any controllers that send redundant input report with different report
    // IDs. If any such devices exist, this may be changed to use a separate buffer to store
    // the last input report for each report ID.
    if (bytesRead == m_lastPollSize &&
            memcmp(pCurrentBuffer, pPreviousBuffer, bytesRead) == 0) {
        return;
    }
    // Cycle between buffers so the memcmp above does not require deep copying to another buffer.
    m_pollingBufferIndex = (m_pollingBufferIndex + 1) % kNumBuffers;
    m_lastPollSize = bytesRead;
    auto incomingData = QByteArray::fromRawData(
            reinterpret_cast<char*>(pCurrentBuffer), bytesRead);

    // Execute callback function in JavaScript mapping
    // and print to stdout in case of --controllerDebug
    emit receive(incomingData, mixxx::Time::elapsed());
}

QByteArray HidIo::getInputReport(unsigned int reportID) {
    auto startOfHidGetInputReport = mixxx::Time::elapsed();
    int bytesRead;

    m_pPollData[m_pollingBufferIndex][0] = reportID;
    // FIXME: implement upstream for hidraw backend on Linux
    // https://github.com/libusb/hidapi/issues/259
    bytesRead = hid_get_input_report(m_pHidDevice, m_pPollData[m_pollingBufferIndex], kBufferSize);
    qCDebug(m_logInput) << bytesRead << "bytes received by hid_get_input_report"
                        << m_deviceInfo.formatName() << "serial #"
                        << m_deviceInfo.serialNumber()
                        << "(including one byte for the report ID:"
                        << QString::number(static_cast<quint8>(reportID), 16)
                                   .toUpper()
                                   .rightJustified(2, QChar('0'))
                        << ") - Needed: "
                        << (mixxx::Time::elapsed() - startOfHidGetInputReport)
                                   .formatMicrosWithUnit();

    if (bytesRead <= kReportIdSize) {
        // -1 is the only error value according to hidapi documentation.
        // Otherwise minimum possible value is 1, because 1 byte is for the reportID,
        // the smallest report with data is therefore 2 bytes.
        DEBUG_ASSERT(bytesRead <= kReportIdSize);
        return QByteArray();
    }

    return QByteArray::fromRawData(
            reinterpret_cast<char*>(m_pPollData[m_pollingBufferIndex]), bytesRead);
}

void HidIo::sendOutputReport(const QByteArray& data, unsigned int reportID) {
    if (m_outputReports.find(reportID) == m_outputReports.end()) {
        std::unique_ptr<HidIoReport> pNewOutputReport;
        m_outputReports[reportID] = std::make_unique<HidIoReport>(
                reportID, m_pHidDevice, std::move(m_deviceInfo), m_logOutput);
    }
    // SendOutputReports executes a hardware operation, which take several milliseconds
    m_outputReports[reportID]->sendOutputReport(data);

    // Ensure that all InputReports are read from the ring buffer, before the next OutputReport blocks the IO again
    poll(); // Polling available Input-Reports is a cheap software only operation, which takes insignificiant time
}

void HidIo::sendFeatureReport(
        const QByteArray& reportData, unsigned int reportID) {
    auto startOfHidSendFeatureReport = mixxx::Time::elapsed();
    QByteArray dataArray;
    dataArray.reserve(kReportIdSize + reportData.size());

    // Append the Report ID to the beginning of dataArray[] per the API..
    dataArray.append(reportID);

    for (const int datum : reportData) {
        dataArray.append(datum);
    }

    int result = hid_send_feature_report(m_pHidDevice,
            reinterpret_cast<const unsigned char*>(dataArray.constData()),
            dataArray.size());
    if (result == -1) {
        qCWarning(m_logOutput)
                << "sendFeatureReport is unable to send data to"
                << m_deviceInfo.formatName() << "serial #"
                << m_deviceInfo.serialNumber() << ":"
                << mixxx::convertWCStringToQString(
                           hid_error(m_pHidDevice), kMaxHidErrorMessageSize);
    } else {
        qCDebug(m_logOutput)
                << result << "bytes sent by sendFeatureReport to"
                << m_deviceInfo.formatName() << "serial #"
                << m_deviceInfo.serialNumber() << "(including report ID of"
                << reportID << ") - Needed: "
                << (mixxx::Time::elapsed() - startOfHidSendFeatureReport)
                           .formatMicrosWithUnit();
    }
}

QByteArray HidIo::getFeatureReport(
        unsigned int reportID) {
    auto startOfHidGetFeatureReport = mixxx::Time::elapsed();
    unsigned char dataRead[kReportIdSize + kBufferSize];
    dataRead[0] = reportID;

    int bytesRead;
    bytesRead = hid_get_feature_report(m_pHidDevice,
            dataRead,
            kReportIdSize + kBufferSize);
    if (bytesRead <= kReportIdSize) {
        // -1 is the only error value according to hidapi documentation.
        // Otherwise minimum possible value is 1, because 1 byte is for the reportID,
        // the smallest report with data is therefore 2 bytes.
        qCWarning(m_logInput)
                << "getFeatureReport is unable to get data from"
                << m_deviceInfo.formatName() << "serial #"
                << m_deviceInfo.serialNumber() << ":"
                << mixxx::convertWCStringToQString(
                           hid_error(m_pHidDevice), kMaxHidErrorMessageSize);
    } else {
        qCDebug(m_logInput)
                << bytesRead << "bytes received by getFeatureReport from"
                << m_deviceInfo.formatName() << "serial #"
                << m_deviceInfo.serialNumber()
                << "(including one byte for the report ID:"
                << QString::number(static_cast<quint8>(reportID), 16)
                           .toUpper()
                           .rightJustified(2, QChar('0'))
                << ") - Needed: "
                << (mixxx::Time::elapsed() - startOfHidGetFeatureReport)
                           .formatMicrosWithUnit();
    }

    // Convert array of bytes read in a JavaScript compatible return type
    // For compatibility with input array HidController::sendFeatureReport, a reportID prefix is not added here
    QByteArray byteArray;
    byteArray.reserve(bytesRead - kReportIdSize);
    const auto* const featureReportStart = reinterpret_cast<const char*>(dataRead + kReportIdSize);
    return QByteArray(featureReportStart, bytesRead);
}
