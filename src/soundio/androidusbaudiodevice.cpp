#include "soundio/androidusbaudiodevice.h"

#include <QtCore/private/qandroidextras_p.h>
#include <libusb.h>
#include <pthread.h>
#include <sys/resource.h>
#include <unistd.h>

#include <QDebug>
#include <QJniEnvironment>
#include <QSet>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstring>

#include "controllers/android.h"

namespace mixxx {
namespace androidusb {

namespace {

constexpr uint8_t kRequestCur = 0x01;
constexpr uint8_t kRequestRange = 0x02;
// UAC1 has separate request codes for reading
constexpr uint8_t kRequestGetCurUac1 = 0x81;
// UAC1 endpoint control / UAC2 clock source control
constexpr uint16_t kSamplingFreqControl = 0x0100;

constexpr int kStopTimeoutMillis = 1000;
constexpr int kControlTimeoutMillis = 1000;
constexpr int kFeedbackTransfers = 2;
constexpr int kFeedbackPacketsPerTransfer = 4;
constexpr int kFeedbackPacketSize = 4;
constexpr size_t kMaxImplicitQueue = 1024;

const int kCommonSampleRates[] = {44100, 48000, 88200, 96000, 32000, 176400, 192000};

std::mutex s_registryMutex;
std::vector<std::shared_ptr<AndroidUsbAudioDevice>> s_registry;
// USB device names (e.g. /dev/bus/usb/001/005) for which the permission was
// already requested, to not ask again on every rescan. A device gets a new
// name when it is plugged in again.
QSet<QString> s_permissionRequested;

QJniObject usbManager() {
    QJniObject context = QNativeInterface::QAndroidApplication::context();
    QJniObject usbService = QJniObject::getStaticObjectField(
            "android/content/Context", "USB_SERVICE", "Ljava/lang/String;");
    return context.callObjectMethod("getSystemService",
            "(Ljava/lang/String;)Ljava/lang/Object;",
            usbService.object());
}

/// Cheap check through the Java API, which doesn't require the USB permission
bool hasAudioStreamingInterface(const QJniObject& usbDevice) {
    const jint interfaceCount = usbDevice.callMethod<jint>("getInterfaceCount");
    for (jint i = 0; i < interfaceCount; ++i) {
        QJniObject usbInterface = usbDevice.callObjectMethod("getInterface",
                "(I)Landroid/hardware/usb/UsbInterface;",
                i);
        // UsbConstants.USB_CLASS_AUDIO = 1, audio streaming subclass = 2
        if (usbInterface.callMethod<jint>("getInterfaceClass") == 1 &&
                usbInterface.callMethod<jint>("getInterfaceSubclass") == 2) {
            return true;
        }
    }
    return false;
}

uint32_t readLittleEndian(const uint8_t* p, int bytes) {
    uint32_t value = 0;
    for (int i = bytes - 1; i >= 0; --i) {
        value = (value << 8) | p[i];
    }
    return value;
}

void raiseThreadPriority() {
    // THREAD_PRIORITY_URGENT_AUDIO; best effort, Android may refuse it
    if (setpriority(PRIO_PROCESS, gettid(), -19) != 0) {
        setpriority(PRIO_PROCESS, gettid(), -16);
    }
#if defined(__aarch64__)
    // The audio engine runs on this thread: enable flush-to-zero (FPCR bit
    // 24) to avoid the denormal performance penalty, like the other sound
    // devices do.
    int64_t fpcr;
    asm volatile("mrs %[fpcr], FPCR" : [fpcr] "=r"(fpcr));
    asm volatile("msr FPCR, %[src]" : : [src] "r"(fpcr | (1ULL << 24)));
#endif
}

} // namespace

AndroidUsbAudioDevice::AndroidUsbAudioDevice(const QJniObject& usbDevice)
        : m_usbDevice(usbDevice),
          m_pContext(nullptr),
          m_pHandle(nullptr),
          m_outputFormat(),
          m_implicitFormat(),
          m_hasImplicitFeedback(false),
          m_claimed(false),
          m_disconnected(false),
          m_eventThreadRunning(false),
          m_outputRunning(false),
          m_outputTransfersPending(0),
          m_framesInFlight(0),
          m_sampleRate(0),
          m_packetsPerSecond(0),
          m_frameAccumulator(0),
          m_nominalFramesPerPacket(0),
          m_framesPerPacket(0),
          m_fractionalFrames(0),
          m_feedbackTransfersPending(0),
          m_measuredFrames16_16(0),
          m_measuredPackets(0),
          m_clockLogged(false),
          m_midiInRunning(false),
          m_midiInPending(false),
          m_pMidiInTransfer(nullptr) {
    m_productName = usbDevice
                            .callObjectMethod("getProductName",
                                    "()Ljava/lang/CharSequence;")
                            .toString();
    m_manufacturerName = usbDevice
                                 .callObjectMethod("getManufacturerName",
                                         "()Ljava/lang/CharSequence;")
                                 .toString();
    m_usbDeviceName =
            usbDevice.callObjectMethod("getDeviceName", "()Ljava/lang/String;")
                    .toString();
    m_vendorId = static_cast<uint16_t>(usbDevice.callMethod<jint>("getVendorId"));
    m_productId = static_cast<uint16_t>(usbDevice.callMethod<jint>("getProductId"));
}

AndroidUsbAudioDevice::~AndroidUsbAudioDevice() {
    stopOutput();
    stopMidiInput();
    stopEventThread();
    if (m_pHandle) {
        libusb_close(m_pHandle);
    }
    if (m_pContext) {
        libusb_exit(m_pContext);
    }
}

std::vector<std::shared_ptr<AndroidUsbAudioDevice>> AndroidUsbAudioDevice::scan(
        DirectMode mode, bool waitForPermission) {
    std::lock_guard lock(s_registryMutex);
    s_registry.clear();
    s_permissionRequested.clear();

    QJniObject manager = usbManager();
    if (!manager.isValid()) {
        qWarning() << "UsbManager not available";
        return {};
    }

    QJniObject deviceList = manager.callObjectMethod("getDeviceList",
            "()Landroid/hardware/usb/UsbDevice;");
    if (!deviceList.isValid()) {
        return {};
    }

    jint count = QJniEnvironment::getJNIEnv()->GetArrayLength(deviceList.object<jobjectArray>());
    for (jint i = 0; i < count; ++i) {
        QJniObject usbDevice = QJniEnvironment::getJNIEnv()->GetObjectArrayElement(
                deviceList.object<jobjectArray>(), i);
        if (!hasAudioStreamingInterface(usbDevice)) {
            continue;
        }

        auto pDevice = std::make_shared<AndroidUsbAudioDevice>(usbDevice);
        if (pDevice->open(mode, waitForPermission)) {
            s_registry.push_back(pDevice);
        }
    }
    return s_registry;
}

std::vector<std::shared_ptr<AndroidUsbAudioDevice>> AndroidUsbAudioDevice::devices() {
    std::lock_guard lock(s_registryMutex);
    std::vector<std::shared_ptr<AndroidUsbAudioDevice>> result;
    for (const auto& pDevice : s_registry) {
        if (pDevice->isConnected()) {
            result.push_back(pDevice);
        }
    }
    return result;
}

bool AndroidUsbAudioDevice::open(DirectMode mode, bool waitForPermission) {
    if (libusb_init(&m_pContext) < 0) {
        qWarning() << "libusb_init failed";
        return false;
    }
    libusb_set_option(m_pContext, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_WARNING);

    QJniObject manager = usbManager();
    if (!manager.isValid()) {
        return false;
    }

    QJniObject connection = manager.callObjectMethod("openDevice",
            "(Landroid/hardware/usb/UsbDevice;)Landroid/hardware/usb/UsbDeviceConnection;",
            m_usbDevice.object());
    if (!connection.isValid()) {
        if (waitForPermission && !s_permissionRequested.contains(m_usbDeviceName)) {
            s_permissionRequested.insert(m_usbDeviceName);
            QNativeInterface::QAndroidApplication::runOnAndroidMainThread([this]() {
                mixxx::android::getIntent();
                QJniObject manager = usbManager();
                if (manager.isValid()) {
                    manager.callMethod<void>("requestPermission",
                            "(Landroid/hardware/usb/UsbDevice;Landroid/app/"
                            "PendingIntent;)V",
                            m_usbDevice.object<jobject>(),
                            mixxx::android::getIntent().object<jobject>());
                }
            }).waitForFinished();
        }
        return false;
    }

    m_connection = connection;
    int fd = connection.callMethod<jint>("getFileDescriptor");
    if (fd < 0) {
        return false;
    }

    if (libusb_wrap_sys_device(m_pContext, static_cast<intptr_t>(fd), &m_pHandle) < 0) {
        qWarning() << "libusb_wrap_sys_device failed";
        return false;
    }

    // Parse descriptors
    QJniObject rawDescriptors = connection.callObjectMethod("getRawDescriptors", "()[B");
    if (rawDescriptors.isValid()) {
        jbyte* bytes = QJniEnvironment::getJNIEnv()->GetByteArrayElements(
                rawDescriptors.object<jbyteArray>(), nullptr);
        jsize length = QJniEnvironment::getJNIEnv()->GetArrayLength(
                rawDescriptors.object<jbyteArray>());
        m_description = usbaudio::parse(
                reinterpret_cast<const uint8_t*>(bytes), length);
        QJniEnvironment::getJNIEnv()->ReleaseByteArrayElements(
                rawDescriptors.object<jbyteArray>(), bytes, JNI_ABORT);
    }

    const usbaudio::StreamFormat* pFormat = usbaudio::chooseOutput(m_description);
    if (!pFormat) {
        qWarning() << "No usable output format for" << m_productName;
        return false;
    }
    m_outputFormat = *pFormat;

    if (m_outputFormat.syncType == usbaudio::SyncType::Asynchronous &&
            m_outputFormat.feedbackEndpoint == 0) {
        const usbaudio::StreamFormat* pImplicit =
                usbaudio::implicitFeedbackSource(m_description, m_outputFormat);
        if (pImplicit) {
            m_implicitFormat = *pImplicit;
            m_hasImplicitFeedback = true;
        }
    }

    if (m_outputFormat.uacVersion == 2) {
        queryUac2SampleRates();
    }

    // Determine if we should drive this device directly
    bool shouldDriveDirectly = false;
    if (mode == DirectMode::Always) {
        shouldDriveDirectly = true;
    } else if (mode == DirectMode::Auto && m_outputFormat.channels > 2) {
        shouldDriveDirectly = true;
    }

    if (shouldDriveDirectly && !claim()) {
        qWarning() << "Failed to claim device" << m_productName;
        return false;
    }

    return true;
}

bool AndroidUsbAudioDevice::claim() {
    if (m_claimed) {
        return true;
    }
    // Claim control interface
    if (!claimInterface(m_outputFormat.interfaceNumber)) {
        return false;
    }
    // Claim implicit feedback interface if different
    if (m_hasImplicitFeedback &&
            m_implicitFormat.interfaceNumber != m_outputFormat.interfaceNumber) {
        if (!claimInterface(m_implicitFormat.interfaceNumber)) {
            return false;
        }
    }
    // Claim MIDI interface if present
    if (m_description.midi.isValid()) {
        claimInterface(m_description.midi.interfaceNumber);
    }
    m_claimed = true;
    return true;
}

bool AndroidUsbAudioDevice::claimInterface(int interfaceNumber) {
    int result = libusb_claim_interface(m_pHandle, interfaceNumber);
    if (result == LIBUSB_ERROR_BUSY) {
        // Detach kernel driver
        if (libusb_kernel_driver_active(m_pHandle, interfaceNumber) == 1) {
            if (libusb_detach_kernel_driver(m_pHandle, interfaceNumber) < 0) {
                qWarning() << "Failed to detach kernel driver from interface" << interfaceNumber;
                return false;
            }
            result = libusb_claim_interface(m_pHandle, interfaceNumber);
        }
    }
    if (result < 0) {
        qWarning() << "Failed to claim interface" << interfaceNumber << ":"
                   << libusb_error_name(result);
        return false;
    }
    m_claimedInterfaces.push_back(interfaceNumber);
    return true;
}

void AndroidUsbAudioDevice::queryUac2SampleRates() {
    // UAC2 clock source sample rate queries (simplified)
    m_outputFormat.sampleRates.clear();
    for (int rate : kCommonSampleRates) {
        if (m_outputFormat.supportsRate(rate)) {
            m_outputFormat.sampleRates.push_back(rate);
        }
    }
    if (m_outputFormat.sampleRates.empty()) {
        m_outputFormat.sampleRates = {44100, 48000};
    }
}

bool AndroidUsbAudioDevice::setSampleRate(const usbaudio::StreamFormat& format, int sampleRate) {
    if (!m_claimed)
        return false;

    uint8_t data[4];
    data[0] = sampleRate & 0xff;
    data[1] = (sampleRate >> 8) & 0xff;
    data[2] = (sampleRate >> 16) & 0xff;
    data[3] = (sampleRate >> 24) & 0xff;

    int request = (format.uacVersion == 1) ? kRequestGetCurUac1 : kRequestCur;
    int result = libusb_control_transfer(m_pHandle,
            LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_ENDPOINT,
            request,
            kSamplingFreqControl,
            format.endpoint,
            data,
            4,
            kControlTimeoutMillis);
    return result == 4;
}

int AndroidUsbAudioDevice::readSampleRate(const usbaudio::StreamFormat& format) {
    uint8_t data[4] = {0};
    int request = (format.uacVersion == 1) ? kRequestGetCurUac1 : kRequestCur;
    int result = libusb_control_transfer(m_pHandle,
            LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_ENDPOINT,
            request,
            kSamplingFreqControl,
            format.endpoint,
            data,
            4,
            kControlTimeoutMillis);
    if (result == 4) {
        return readLittleEndian(data, 4);
    }
    return 0;
}

void AndroidUsbAudioDevice::applySampleRate(int sampleRate) {
    if (setSampleRate(m_outputFormat, sampleRate)) {
        m_sampleRate = sampleRate;
    }
    if (m_hasImplicitFeedback) {
        setSampleRate(m_implicitFormat, sampleRate);
    }
}

void AndroidUsbAudioDevice::measureClock(uint64_t framesPerPacket16_16) {
    m_measuredFrames16_16 += framesPerPacket16_16;
    ++m_measuredPackets;
    if (m_measuredPackets >= m_packetsPerSecond) {
        double measuredRate = static_cast<double>(m_measuredFrames16_16) / 65536.0;
        qInfo() << "Measured device clock for" << m_productName << ":" << measuredRate << "Hz";
        m_clockLogged = true;
        m_measuredFrames16_16 = 0;
        m_measuredPackets = 0;
    }
}

bool AndroidUsbAudioDevice::startOutput(int sampleRate,
        int transferMillis,
        int numTransfers,
        RenderCallback renderCallback) {
    if (m_outputRunning) {
        return true;
    }
    if (!m_claimed) {
        if (!claim()) {
            return false;
        }
    }

    applySampleRate(sampleRate);

    // Set altsetting
    if (libusb_set_interface_alt_setting(m_pHandle,
                m_outputFormat.interfaceNumber,
                m_outputFormat.altSetting) < 0) {
        qWarning() << "Failed to set altsetting for output";
        return false;
    }
    if (m_hasImplicitFeedback &&
            m_implicitFormat.interfaceNumber != m_outputFormat.interfaceNumber) {
        if (libusb_set_interface_alt_setting(m_pHandle,
                    m_implicitFormat.interfaceNumber,
                    m_implicitFormat.altSetting) < 0) {
            qWarning() << "Failed to set altsetting for implicit feedback";
            return false;
        }
    }

    m_renderCallback = std::move(renderCallback);
    m_sampleRate = sampleRate;
    m_packetsPerSecond = packetsPerSecond(m_outputFormat.bInterval, isHighSpeed());
    m_frameAccumulator = 0;

    // Calculate nominal frames per packet in 16.16
    double framesPerPacket = static_cast<double>(sampleRate) / m_packetsPerSecond;
    m_nominalFramesPerPacket = static_cast<uint32_t>(framesPerPacket * 65536.0 + 0.5);

    // Prepare output transfers
    const int framesPerTransfer = (sampleRate * transferMillis + 999) / 1000;
    const int packetSize = m_outputFormat.maxPacketSize;
    const int bytesPerFrame = m_outputFormat.frameBytes();
    const int transferSize = framesPerTransfer * bytesPerFrame;

    m_outputTransfers.clear();
    m_outputTransfers.reserve(numTransfers);
    for (int i = 0; i < numTransfers; ++i) {
        libusb_transfer* transfer = libusb_alloc_transfer(
                framesPerTransfer * 2 / m_outputFormat.maxPacketSize + 2);
        auto* buffer = static_cast<unsigned char*>(std::calloc(transferSize, 1));
        libusb_fill_iso_transfer(transfer,
                m_pHandle,
                m_outputFormat.endpoint,
                buffer,
                transferSize,
                framesPerTransfer,
                outputTransferCallback,
                this,
                0);
        transfer->flags = LIBUSB_TRANSFER_FREE_BUFFER;
        m_outputTransfers.push_back(transfer);
    }

    // Prepare feedback transfers for async endpoints
    if (m_outputFormat.syncType == usbaudio::SyncType::Asynchronous) {
        m_feedbackTransfers.clear();
        m_feedbackTransfers.reserve(kFeedbackTransfers);
        for (int i = 0; i < kFeedbackTransfers; ++i) {
            libusb_transfer* transfer = libusb_alloc_transfer(kFeedbackPacketsPerTransfer);
            auto* buffer = static_cast<unsigned char*>(std::calloc(
                    kFeedbackPacketsPerTransfer * kFeedbackPacketSize, 1));
            libusb_fill_iso_transfer(transfer,
                    m_pHandle,
                    m_outputFormat.feedbackEndpoint
                            ? m_outputFormat.feedbackEndpoint
                            : m_implicitFormat.endpoint,
                    buffer,
                    kFeedbackPacketsPerTransfer * kFeedbackPacketSize,
                    kFeedbackPacketsPerTransfer,
                    m_outputFormat.feedbackEndpoint ? feedbackTransferCallback
                                                    : implicitTransferCallback,
                    this,
                    0);
            transfer->flags = LIBUSB_TRANSFER_FREE_BUFFER;
            m_feedbackTransfers.push_back(transfer);
        }
    }

    m_outputRunning = true;
    m_outputTransfersPending = 0;
    m_framesInFlight = 0;
    m_frameAccumulator = 0;
    m_fractionalFrames = 0;
    m_clockLogged = false;
    m_measuredFrames16_16 = 0;
    m_measuredPackets = 0;

    // Submit initial silent transfers
    for (auto* transfer : m_outputTransfers) {
        fillOutputTransfer(transfer, true);
        if (libusb_submit_transfer(transfer) == 0) {
            ++m_outputTransfersPending;
        }
    }
    for (auto* transfer : m_feedbackTransfers) {
        if (libusb_submit_transfer(transfer) == 0) {
            ++m_feedbackTransfersPending;
        }
    }

    startEventThread();
    return true;
}

void AndroidUsbAudioDevice::fillOutputTransfer(libusb_transfer* pTransfer, bool silent) {
    int numPackets = pTransfer->num_iso_packets;
    int offset = 0;
    for (int i = 0; i < numPackets; ++i) {
        int frames = nextPacketFrames();
        int bytes = frames * m_outputFormat.frameBytes();
        libusb_iso_packet_descriptor* desc = &pTransfer->iso_packet_desc[i];
        desc->offset = offset;
        desc->length = bytes;
        offset += bytes;
        if (silent) {
            std::fill_n(reinterpret_cast<uint8_t*>(pTransfer->buffer) + desc->offset, bytes, 0);
        } else if (m_renderCallback) {
            m_renderCallback(reinterpret_cast<uint8_t*>(pTransfer->buffer) + desc->offset, frames);
        }
    }
    pTransfer->length = offset;
}

int AndroidUsbAudioDevice::nextPacketFrames() {
    if (m_outputFormat.syncType == usbaudio::SyncType::Asynchronous) {
        // Use feedback-driven packet size
        uint32_t frames16_16 = m_framesPerPacket;
        if (!m_clockLogged && m_measuredPackets > 0) {
            measureClock(m_framesPerPacket);
        }
        return static_cast<int>((frames16_16 + m_fractionalFrames) >> 16);
    }
    // Synchronous/Adaptive: fixed packet size
    double framesPerPacket = static_cast<double>(m_sampleRate) / m_packetsPerSecond;
    return static_cast<int>(framesPerPacket + 0.5);
}

void AndroidUsbAudioDevice::cancelAndFree(
        std::vector<libusb_transfer*>* pTransfers, std::atomic<int>* pPending) {
    for (auto* transfer : *pTransfers) {
        libusb_cancel_transfer(transfer);
    }
    const auto deadline = std::chrono::steady_clock::now() +
            std::chrono::milliseconds(kStopTimeoutMillis);
    while (*pPending > 0 && std::chrono::steady_clock::now() < deadline) {
        if (m_eventThreadRunning) {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        } else {
            timeval timeout{0, 10000};
            libusb_handle_events_timeout_completed(m_pContext, &timeout, nullptr);
        }
    }
    if (*pPending > 0) {
        qWarning() << "USB transfers of" << m_productName << "did not finish, leaking them";
    } else {
        for (libusb_transfer* pTransfer : *pTransfers) {
            libusb_free_transfer(pTransfer);
        }
    }
    pTransfers->clear();
    *pPending = 0;
}

void AndroidUsbAudioDevice::stopOutput() {
    std::lock_guard lock(m_mutex);
    if (m_outputTransfers.empty() && m_feedbackTransfers.empty()) {
        return;
    }
    m_outputRunning = false;
    cancelAndFree(&m_outputTransfers, &m_outputTransfersPending);
    cancelAndFree(&m_feedbackTransfers, &m_feedbackTransfersPending);
    m_framesInFlight = 0;
    if (!m_disconnected) {
        // Zero bandwidth altsettings
        libusb_set_interface_alt_setting(m_pHandle, m_outputFormat.interfaceNumber, 0);
        if (m_hasImplicitFeedback &&
                m_implicitFormat.interfaceNumber != m_outputFormat.interfaceNumber) {
            libusb_set_interface_alt_setting(m_pHandle, m_implicitFormat.interfaceNumber, 0);
        }
    }
}

bool AndroidUsbAudioDevice::startMidiInput(MidiCallback callback) {
    std::lock_guard lock(m_mutex);
    if (m_midiInRunning) {
        return true;
    }
    const usbaudio::MidiInterface& midi = m_description.midi;
    if (!midi.isValid() || midi.inEndpoint == 0 || !claim()) {
        return false;
    }
    m_midiCallback = std::move(callback);
    if (!m_pMidiInTransfer) {
        m_pMidiInTransfer = libusb_alloc_transfer(0);
        const int size = std::max(64, midi.inMaxPacketSize);
        auto* pBuffer = static_cast<unsigned char*>(std::calloc(size, 1));
        libusb_fill_bulk_transfer(m_pMidiInTransfer,
                m_pHandle,
                midi.inEndpoint,
                pBuffer,
                size,
                midiInTransferCallback,
                this,
                0);
        m_pMidiInTransfer->flags = LIBUSB_TRANSFER_FREE_BUFFER;
    }
    m_midiInRunning = true;
    if (libusb_submit_transfer(m_pMidiInTransfer) < 0) {
        m_midiInRunning = false;
        return false;
    }
    m_midiInPending = true;
    return true;
}

void AndroidUsbAudioDevice::midiInTransferCallback(libusb_transfer* pTransfer) {
    auto* pThis = static_cast<AndroidUsbAudioDevice*>(pTransfer->user_data);
    if (pTransfer->status == LIBUSB_TRANSFER_COMPLETED) {
        for (int i = 0; i + 4 <= pTransfer->actual_length; i += 4) {
            const uint8_t* pPacket = pTransfer->buffer + i;
            if (pPacket[0] != 0 && pThis->m_midiCallback) {
                pThis->m_midiCallback(pPacket);
            }
        }
    } else if (pTransfer->status == LIBUSB_TRANSFER_NO_DEVICE) {
        pThis->markDisconnected();
    }
    if (pThis->m_midiInRunning && pTransfer->status != LIBUSB_TRANSFER_CANCELLED &&
            pTransfer->status != LIBUSB_TRANSFER_NO_DEVICE &&
            libusb_submit_transfer(pTransfer) == 0) {
        return;
    }
    pThis->m_midiInPending = false;
}

void AndroidUsbAudioDevice::stopMidiInput() {
    std::lock_guard lock(m_mutex);
    if (!m_pMidiInTransfer) {
        return;
    }
    m_midiInRunning = false;
    if (m_midiInPending) {
        libusb_cancel_transfer(m_pMidiInTransfer);
        const auto deadline = std::chrono::steady_clock::now() +
                std::chrono::milliseconds(kStopTimeoutMillis);
        while (m_midiInPending && std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }
    if (!m_midiInPending) {
        libusb_free_transfer(m_pMidiInTransfer);
    }
    m_pMidiInTransfer = nullptr;
    m_midiCallback = nullptr;
}

bool AndroidUsbAudioDevice::sendMidi(const uint8_t* pData, int length) {
    const uint8_t endpoint = m_description.midi.outEndpoint;
    if (!m_claimed || m_disconnected || endpoint == 0) {
        return false;
    }
    std::vector<unsigned char> buffer(pData, pData + length);
    int transferred = 0;
    const int error = libusb_bulk_transfer(m_pHandle,
            endpoint,
            buffer.data(),
            length,
            &transferred,
            100);
    if (error == LIBUSB_ERROR_NO_DEVICE) {
        markDisconnected();
    }
    return error == 0 && transferred == length;
}

void AndroidUsbAudioDevice::startEventThread() {
    if (m_eventThreadRunning) {
        return;
    }
    m_eventThreadRunning = true;
    m_eventThread = std::thread([this]() {
        raiseThreadPriority();
        while (m_eventThreadRunning) {
            timeval timeout{0, 10000}; // 10ms
            libusb_handle_events_timeout_completed(m_pContext, &timeout, nullptr);
        }
    });
}

void AndroidUsbAudioDevice::stopEventThread() {
    if (!m_eventThreadRunning) {
        return;
    }
    m_eventThreadRunning = false;
    if (m_eventThread.joinable()) {
        m_eventThread.join();
    }
}

void AndroidUsbAudioDevice::markDisconnected() {
    m_disconnected = true;
    stopOutput();
    stopMidiInput();
    stopEventThread();
}

bool AndroidUsbAudioDevice::isHighSpeed() const {
    // libusb doesn't expose speed directly; assume high speed for now
    return true;
}

void AndroidUsbAudioDevice::outputTransferCallback(libusb_transfer* pTransfer) {
    auto* pThis = static_cast<AndroidUsbAudioDevice*>(pTransfer->user_data);
    if (pTransfer->status == LIBUSB_TRANSFER_COMPLETED) {
        --pThis->m_outputTransfersPending;
        pThis->m_framesInFlight += pTransfer->actual_length / pThis->m_outputFormat.frameBytes();
        if (pThis->m_outputRunning) {
            pThis->fillOutputTransfer(pTransfer, false);
            if (libusb_submit_transfer(pTransfer) == 0) {
                ++pThis->m_outputTransfersPending;
                return;
            }
        }
    } else if (pTransfer->status == LIBUSB_TRANSFER_NO_DEVICE) {
        pThis->markDisconnected();
    }
    if (!pThis->m_outputRunning) {
        libusb_free_transfer(pTransfer);
    }
}

void AndroidUsbAudioDevice::feedbackTransferCallback(libusb_transfer* pTransfer) {
    auto* pThis = static_cast<AndroidUsbAudioDevice*>(pTransfer->user_data);
    if (pTransfer->status == LIBUSB_TRANSFER_COMPLETED) {
        --pThis->m_feedbackTransfersPending;
        for (int i = 0; i < pTransfer->num_iso_packets; ++i) {
            const libusb_iso_packet_descriptor* desc = &pTransfer->iso_packet_desc[i];
            if (desc->status == 0 && desc->actual_length == 4) {
                const uint32_t raw = *reinterpret_cast<const uint32_t*>(
                        static_cast<const uint8_t*>(pTransfer->buffer) + desc->offset);
                const uint32_t frames = usbaudio::feedbackToFramesPerPacket(
                        raw, 4, pThis->m_nominalFramesPerPacket);
                if (frames > 0) {
                    pThis->m_framesPerPacket = frames;
                    pThis->m_fractionalFrames = 0;
                }
            }
        }
        if (pThis->m_outputRunning) {
            if (libusb_submit_transfer(pTransfer) == 0) {
                ++pThis->m_feedbackTransfersPending;
                return;
            }
        }
    } else if (pTransfer->status == LIBUSB_TRANSFER_NO_DEVICE) {
        pThis->markDisconnected();
    }
    if (!pThis->m_outputRunning) {
        libusb_free_transfer(pTransfer);
    }
}

void AndroidUsbAudioDevice::implicitTransferCallback(libusb_transfer* pTransfer) {
    auto* pThis = static_cast<AndroidUsbAudioDevice*>(pTransfer->user_data);
    if (pTransfer->status == LIBUSB_TRANSFER_COMPLETED) {
        --pThis->m_feedbackTransfersPending;
        for (int i = 0; i < pTransfer->num_iso_packets; ++i) {
            const libusb_iso_packet_descriptor* desc = &pTransfer->iso_packet_desc[i];
            if (desc->status == 0) {
                int frames = desc->actual_length / pThis->m_implicitFormat.frameBytes();
                pThis->m_implicitPacketFrames.push_back(frames);
                if (static_cast<int>(pThis->m_implicitPacketFrames.size()) > kMaxImplicitQueue) {
                    pThis->m_implicitPacketFrames.pop_front();
                }
            }
        }
        if (!pThis->m_implicitPacketFrames.empty()) {
            int sum = 0;
            for (int f : pThis->m_implicitPacketFrames)
                sum += f;
            uint32_t avg16_16 = static_cast<uint32_t>(
                    (static_cast<double>(sum) /
                            pThis->m_implicitPacketFrames.size()) *
                            65536.0 +
                    0.5);
            pThis->m_framesPerPacket = avg16_16;
            pThis->m_fractionalFrames = 0;
        }
        if (pThis->m_outputRunning) {
            if (libusb_submit_transfer(pTransfer) == 0) {
                ++pThis->m_feedbackTransfersPending;
                return;
            }
        }
    } else if (pTransfer->status == LIBUSB_TRANSFER_NO_DEVICE) {
        pThis->markDisconnected();
    }
    if (!pThis->m_outputRunning) {
        libusb_free_transfer(pTransfer);
    }
}

QList<int> AndroidUsbAudioDevice::sampleRates() const {
    if (!m_outputFormat.sampleRates.empty()) {
        return m_outputFormat.sampleRates;
    }
    return {44100, 48000};
}

} // namespace androidusb
} // namespace mixxx
