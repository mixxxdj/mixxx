#include "controllers/hid/hidiothread.h"

#include "util/assert.h"

#ifdef __ANDROID__
#include <android/log.h>
#include <hidapi_libusb.h>
#else
#include <hidapi.h>
#endif
#include "moc_hidiothread.cpp"
#include "util/runtimeloggingcategory.h"
#include "util/string.h"
#include "util/time.h"
#include "util/trace.h"

namespace {
constexpr int kReportIdSize = 1;
constexpr size_t kMaxHidErrorMessageSize = 512;

// Sleep time of run loop, in idle case, when no time consuming operation was executed before
// The lower the time the more even the CPU load is spread over time
// High values block the IO of this device and reduce response time and also the bandwidth.
// Note, that this value has no influence on the amount of data to process,
// the rate of InputReports is defined by the HID device itself.
// This time should be below the rate of the HID device, which is ~1kHz for typical DJ controllers
// the fastest possible rate of HID devices with USB HighSpeed or USB SuperSpeed interface is 8kHz
constexpr int kSleepTimeWhenIdleMicros = 250;

QString loggingCategoryPrefix(const QString& deviceName) {
    return QStringLiteral("controller.") +
            RuntimeLoggingCategory::removeInvalidCharsFromCategory(deviceName.toLower());
}
} // namespace

HidIoThread::HidIoThread(hid_device* pHidDevice,
        const mixxx::hid::DeviceInfo& deviceInfo,
        std::optional<bool> deviceUsesReportIds)
        : QThread(),
          m_deviceInfo(deviceInfo),
          // Defining RuntimeLoggingCategories locally in this thread improves
          // runtime performance significantly
          m_logBase(loggingCategoryPrefix(deviceInfo.formatName())),
          m_logInput(loggingCategoryPrefix(deviceInfo.formatName()) +
                  QStringLiteral(".input")),
          m_logOutput(loggingCategoryPrefix(deviceInfo.formatName()) +
                  QStringLiteral(".output")),
          m_pHidDevice(pHidDevice),
          m_lastPollSize(0),
          m_pollingBufferIndex(0),
          m_hidReadErrorLogged(false),
          m_deviceUsesReportIds(deviceUsesReportIds),
          m_globalOutputReportFifo(),
          m_runLoopSemaphore(1) {
    // Initializing isn't strictly necessary but is good practice.
    for (int i = 0; i < kNumBuffers; i++) {
        memset(m_pPollData[i], 0, kBufferSize);
    }
    m_outputReportIterator = m_outputReports.begin();
    m_state.storeRelease(static_cast<int>(HidIoThreadState::Initialized));
}

HidIoThread::~HidIoThread() {
    hid_close(m_pHidDevice);
#ifdef Q_OS_ANDROID
    if (m_androidConnection.isValid()) {
        m_androidConnection.callMethod<void>("close");
    }
#endif
}

void HidIoThread::run() {
    const QSemaphoreReleaser releaser(m_runLoopSemaphore);
    m_runLoopSemaphore.acquire();
    while (!testAndSetThreadState(HidIoThreadState::StopRequested, HidIoThreadState::Stopped)) {
        // Ensure that all InputReports are read from the ring buffer, before the next OutputReport blocks the IO again
        // Polling available Input-Reports is a cheap software only operation, which takes insignificiant time
        pollBufferedInputReports();

        // Send one OutputReport, if at least one is cached
        // Sending an OutputReport is time consuming, because HIDAPI waits
        // for the backend/kernel for confirmation of success
        // Depending on the OS this takes several several milli seconds
        // This operation doesn't take many CPU cycles, most time HIDAPI is in idle state
        if (!sendNextCachedOutputReport()) {
            if (testAndSetThreadState(HidIoThreadState::StopWhenAllReportsSent,
                        HidIoThreadState::Stopped)) {
                break;
            }
            // Sleep run loop, if no OutputReport was send
            // Tests on Windows and Linux showed that the thread schedulers
            // handle usleep wait times reliable under CPU load
            usleep(kSleepTimeWhenIdleMicros);
        }
    }
}

void HidIoThread::pollBufferedInputReports() {
    Trace hidRead("HidIoThread pollBufferedInputReports");
    auto hidDeviceLock = lockMutex(&m_hidDeviceAndPollMutex);
    // This function reads the available HID Input Reports using hidapi.
    // Important to know is, that this reading is not a hardware operation,
    // instead it reads previously received HID Input Reports from a ring buffer.
    // Depending on the hidapi backend implementation the ring buffer is either part
    // of the hidapi implementation or the OS kernel.
    // The size of the ring buffer also depends on the hidapi backend:
    // - hidraw(2048 bytes) - Linux Kernel API
    // - libusb(30 reports) - BSD (alternative Linux userspace implementation)
    // - mac(30 reports)
    // - windows(64 reports)
    // If the interval between two polls is to long, multiple buffered HID InputReports
    // will be processed at the same time.
    while (m_state.loadAcquire() == static_cast<int>(HidIoThreadState::InputOutputActive)) {
        int bytesRead = hid_read(m_pHidDevice, m_pPollData[m_pollingBufferIndex], kBufferSize);
        if (bytesRead < 0) {
            // -1 is the only error value according to hidapi documentation.
            DEBUG_ASSERT(bytesRead == -1);
            if (!m_hidReadErrorLogged) {
                qCWarning(m_logOutput)
                        << "Unable to read buffered HID InputReports from"
                        << m_deviceInfo.formatName() << ":"
                        << mixxx::convertWCStringToQString(
                                   hid_error(m_pHidDevice),
                                   kMaxHidErrorMessageSize)
                        << "Note that, this message is only logged once and "
                           "may not appear again until all hid_read errors "
                           "have disappeared.";
                // Stop logging error messages if every hid_read() fails to avoid large log files
                m_hidReadErrorLogged = true;
            }
            break;
        } else {
            m_hidReadErrorLogged = false; // Allow to log new errors
            if (bytesRead == 0) {
                // No InputReports left to be read
                break;
            }
        }
        processInputReport(bytesRead);
    }
}

void HidIoThread::processInputReport(int bytesRead) {
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

    // Convert array of bytes read in a JavaScript compatible return type, this is emitted as deep-copy, for thread safety.
    // This eexecute callback function in JavaScript mapping and print to stdout in case of --controllerDebug
    emit receive(QByteArray(reinterpret_cast<const char*>(pCurrentBuffer),
                         bytesRead),
            mixxx::Time::elapsed());

    if (m_deviceUsesReportIds.has_value() && bytesRead > 0) {
        if (m_deviceUsesReportIds.value()) {
            // Extract the ReportId from the buffer
            quint8 reportId = pCurrentBuffer[0];
            emit reportReceived(reportId,
                    QByteArray(
                            reinterpret_cast<const char*>(pCurrentBuffer + 1),
                            bytesRead - 1));
        } else {
            quint8 reportId = 0;
            emit reportReceived(reportId,
                    QByteArray(reinterpret_cast<const char*>(pCurrentBuffer),
                            bytesRead));
        }
    }
}

QByteArray HidIoThread::getInputReport(quint8 reportID) {
    auto startOfHidGetInputReport = mixxx::Time::elapsed();
    auto hidDeviceLock = lockMutex(&m_hidDeviceAndPollMutex);

    m_pPollData[m_pollingBufferIndex][0] = reportID;
    int bytesRead = hid_get_input_report(
            m_pHidDevice, m_pPollData[m_pollingBufferIndex], kBufferSize);
    if (bytesRead <= kReportIdSize) {
        // -1 is the only error value according to hidapi documentation.
        // Otherwise minimum possible value is 1, because 1 byte is for the reportID,
        // the smallest report with data is therefore 2 bytes
        qCWarning(m_logInput)
                << "getInputReport is unable to get data from"
                << m_deviceInfo.formatName() << "serial #"
                << m_deviceInfo.getSerialNumber() << ":"
                << mixxx::convertWCStringToQString(
                           hid_error(m_pHidDevice), kMaxHidErrorMessageSize);
        // Note, that the GetInputReport request is optional, according to the HID specification,
        // not all devices implement it.
        return {};
    }

    // Convert array of bytes read in a JavaScript compatible return type, this is returned as deep-copy, for thread safety.
    QByteArray returnArray = QByteArray(
            reinterpret_cast<char*>(m_pPollData[m_pollingBufferIndex] + kReportIdSize),
            bytesRead - kReportIdSize);

    hidDeviceLock.unlock();

    qCDebug(m_logInput) << bytesRead << "bytes received by hid_get_input_report"
                        << m_deviceInfo.formatName() << "serial #"
                        << m_deviceInfo.getSerialNumber()
                        << "(including one byte for the report ID:"
                        << QString::number(static_cast<quint8>(reportID), 16)
                                   .toUpper()
                                   .rightJustified(2, QChar('0'))
                        << ") - Needed: "
                        << (mixxx::Time::elapsed() - startOfHidGetInputReport)
                                   .formatMicrosWithUnit();

    return returnArray;
}

void HidIoThread::updateCachedOutputReportData(quint8 reportID,
        const QByteArray& data,
        bool useNonSkippingFIFO) {
    auto mapLock = lockMutex(&m_outputReportMapMutex);
    if (m_outputReports.find(reportID) == m_outputReports.end()) {
        std::unique_ptr<HidIoOutputReport> pNewOutputReport;
        m_outputReports[reportID] = std::make_unique<HidIoOutputReport>(
                reportID, data.size());
    }

    // The only mutable operation on m_outputReports is insert
    // by std::map<Key,T,Compare,Allocator>::operator[]
    // The standard says that "No iterators or references are invalidated." using this operator.
    // Therefore actualOutputReportIterator doesn't require Mutex protection.
    auto actualOutputReportIterator = m_outputReports.find(reportID);

    mapLock.unlock();

    // If useNonSkippingFIFO is false, the report data are cached here
    // If useNonSkippingFIFO is true, this cache is cleared
    actualOutputReportIterator->second->updateCachedData(
            data, m_logOutput, useNonSkippingFIFO);

    // If useNonSkippingFIFO is true, put the new report dataset on the FIFO
    if (useNonSkippingFIFO) {
        m_globalOutputReportFifo.addReportDatasetToFifo(reportID, data, m_deviceInfo, m_logOutput);
    }
}

bool HidIoThread::sendNextCachedOutputReport() {
    // 1.) Send non-skipping reports from FIFO
    if (m_globalOutputReportFifo.sendNextReportDataset(&m_hidDeviceAndPollMutex,
                m_pHidDevice,
                m_deviceInfo,
                m_logOutput)) {
        // Return after each time consuming sendCachedData
        return true;
    }

    // 2.) If non non-skipping reports were in the FIFO, send the skipable reports
    // from the m_outputReports cache

    // m_outputReports.size() doesn't need mutex protection, because the value of i is not used.
    // i is just a counter to prevent infinite loop execution.
    // If the map size increases, this loop will execute one iteration more,
    // which only has the effect, that one additional lookup operation for unsent data will be executed.
    for (std::size_t i = 0; i < m_outputReports.size(); i++) {
        auto mapLock = lockMutex(&m_outputReportMapMutex);
        if (m_outputReportIterator == m_outputReports.end()) {
            m_outputReportIterator = m_outputReports.begin();
        } else {
            m_outputReportIterator++;
            if (m_outputReportIterator == m_outputReports.end()) {
                m_outputReportIterator = m_outputReports.begin();
            }
        }
        mapLock.unlock();

        // The only mutable operation on m_outputReports is insert
        // by std::map<Key,T,Compare,Allocator>::operator[]
        // The standard says that "No iterators or references are invalidated." using this operator.
        // Therefore m_outputReportIterator doesn't require Mutex protection.
        if (m_outputReportIterator->second->sendCachedData(
                    &m_hidDeviceAndPollMutex, m_pHidDevice, m_logOutput)) {
            // Return after each time consuming sendCachedData
            return true;
        }
    }
    // Returns false if no report required a time consuming sendCachedData
    return false;
}

void HidIoThread::sendFeatureReport(
        quint8 reportID, const QByteArray& reportData) {
    auto startOfHidSendFeatureReport = mixxx::Time::elapsed();
    QByteArray dataArray;
    dataArray.reserve(kReportIdSize + reportData.size());

    // Append the Report ID to the beginning of dataArray[] per the API..
    dataArray.append(reportID);
    dataArray.append(reportData);

    auto hidDeviceLock = lockMutex(&m_hidDeviceAndPollMutex);
    int result = hid_send_feature_report(m_pHidDevice,
            reinterpret_cast<const unsigned char*>(dataArray.constData()),
            dataArray.size());
    if (result == -1) {
        qCWarning(m_logOutput)
                << "sendFeatureReport is unable to send data to"
                << m_deviceInfo.formatName() << "serial #"
                << m_deviceInfo.getSerialNumber() << ":"
                << mixxx::convertWCStringToQString(
                           hid_error(m_pHidDevice), kMaxHidErrorMessageSize);
        return;
    }

    hidDeviceLock.unlock();

    qCDebug(m_logOutput)
            << result << "bytes sent by sendFeatureReport to"
            << m_deviceInfo.formatName() << "serial #"
            << m_deviceInfo.getSerialNumber() << "(including report ID of"
            << reportID << ") - Needed: "
            << (mixxx::Time::elapsed() - startOfHidSendFeatureReport)
                       .formatMicrosWithUnit();
}

QByteArray HidIoThread::getFeatureReport(
        quint8 reportID) {
    auto startOfHidGetFeatureReport = mixxx::Time::elapsed();
    unsigned char dataRead[kReportIdSize + kBufferSize];
    dataRead[0] = reportID;

    auto hidDeviceLock = lockMutex(&m_hidDeviceAndPollMutex);
    int bytesRead = hid_get_feature_report(m_pHidDevice,
            dataRead,
            kReportIdSize + kBufferSize);
    if (bytesRead <= kReportIdSize) {
        // -1 is the only error value according to hidapi documentation.
        // Otherwise minimum possible value is 1, because 1 byte is for the reportID,
        // the smallest report with data is therefore 2 bytes.
        qCWarning(m_logInput)
                << "getFeatureReport is unable to get data from"
                << m_deviceInfo.formatName() << "serial #"
                << m_deviceInfo.getSerialNumber() << ":"
                << mixxx::convertWCStringToQString(
                           hid_error(m_pHidDevice), kMaxHidErrorMessageSize);
        return {};
    }

    hidDeviceLock.unlock();

    qCDebug(m_logInput)
            << bytesRead << "bytes received by getFeatureReport from"
            << m_deviceInfo.formatName() << "serial #"
            << m_deviceInfo.getSerialNumber()
            << "(including one byte for the report ID:"
            << QString::number(static_cast<quint8>(reportID), 16)
                       .toUpper()
                       .rightJustified(2, QChar('0'))
            << ") - Needed: "
            << (mixxx::Time::elapsed() - startOfHidGetFeatureReport)
                       .formatMicrosWithUnit();

    // Convert array of bytes read in a JavaScript compatible return type.
    // For compatibility with input array HidController::sendFeatureReport, a reportID prefix is not added here
    return QByteArray(reinterpret_cast<const char*>(dataRead + kReportIdSize),
            bytesRead - kReportIdSize);
}

bool HidIoThread::testAndSetThreadState(HidIoThreadState expectedState,
        HidIoThreadState newState) {
    if (!m_state.testAndSetOrdered(static_cast<int>(expectedState),
                static_cast<int>(newState))) {
        // Test result must be handled outside of this function. [[nodiscard]]
        return false;
    }

    return true;
}

bool HidIoThread::waitUntilRunLoopIsStopped(unsigned int timeoutMillis) {
    DEBUG_ASSERT(timeoutMillis != 0);

    return m_runLoopSemaphore.tryAcquire(1, timeoutMillis);
}

void HidIoThread::setThreadState(HidIoThreadState expectedState) {
    m_state.storeRelease(static_cast<int>(expectedState));
}
